#include "operator_manager.h"
#include "drone_manager.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t add_platform_sig = 0;
volatile sig_atomic_t remove_platform_sig = 0;

void handle_op_signal(int sig)
{
  if (sig == SIG_ADD_PLATFORM)
    add_platform_sig = 1;
  if (sig == SIG_REMOVE_PLATFORM)
    remove_platform_sig = 1;
}

void add_platform(int sem_id, SharedStorage *shm)
{
  // Double Capacity
  lock_sem(sem_id, SEM_SHM_ACCESS);

  // Increase target drone count (Doubling current, up to max 2*N)
  int new_target = shm->base.target_drone_count * 2;
  int limit = shm->base.initial_drone_count * 2;

  if (new_target > limit)
    new_target = limit;
  if (new_target > MAX_DRONES_TOTAL)
    new_target = MAX_DRONES_TOTAL;

  shm->base.target_drone_count = new_target;

  int old_cap = shm->base.max_capacity;
  int new_cap;
  if (new_target == 1)
  {
    new_cap = 0;
  }
  else
  {
    new_cap = (new_target - 1) / 2;
  }

  if (new_cap > MAX_DRONES_TOTAL)
    new_cap = MAX_DRONES_TOTAL;

  int added = new_cap - old_cap;
  shm->base.max_capacity = new_cap;

  unlock_sem(sem_id, SEM_SHM_ACCESS);

  // Post semaphore 'added' times
  struct sembuf sb;
  sb.sem_num = SEM_BASE_CAPACITY;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  for (int i = 0; i < added; ++i)
    semop(sem_id, &sb, 1);

  log_event("Operator added platform. New capacity: %d, Target drones: %d", new_cap, new_target);
}

void remove_platform(int sem_id, SharedStorage *shm)
{
  // Halve Capacity
  lock_sem(sem_id, SEM_SHM_ACCESS);

  int new_target = shm->base.target_drone_count / 2;
  if (new_target < MIN_DRONES)
    new_target = MIN_DRONES;
  shm->base.target_drone_count = new_target;

  int old_cap = shm->base.max_capacity;
  int new_cap;
  if (new_target == 1)
  {
    new_cap = 0;
  }
  else
  {
    new_cap = (new_target - 1) / 2;
  }

  int removed = old_cap - new_cap;
  shm->base.max_capacity = new_cap;

  unlock_sem(sem_id, SEM_SHM_ACCESS);

  struct sembuf sb;
  sb.sem_num = SEM_BASE_CAPACITY;
  sb.sem_op = -1;
  sb.sem_flg = IPC_NOWAIT;

  int pending_removals = 0;

  for (int i = 0; i < removed; ++i)
  {
    if (semop(sem_id, &sb, 1) == -1)
    {
      if (errno == EAGAIN)
      {
        pending_removals++;
      }
      else
      {
        perror("Failed to decrement semaphore in remove_platform");
      }
    }
  }

  if (pending_removals > 0)
  {
    log_event("Operator: %d slots occupied, starting background removal", pending_removals);
    pid_t pid = fork();
    if (pid == 0)
    {
      struct sembuf sb_wait;
      sb_wait.sem_num = SEM_BASE_CAPACITY;
      sb_wait.sem_op = -1;
      sb_wait.sem_flg = 0;

      for (int i = 0; i < pending_removals; ++i)
      {
        if (semop(sem_id, &sb_wait, 1) == -1)
        {
          if (errno == EINTR)
          {
            i--;
            continue;
          }
          if (errno == EIDRM || errno == EINVAL)
          {
            exit(0);
          }
          perror("Helper failed to decrement");
          exit(1);
        }
      }
      exit(0);
    }
    else if (pid < 0)
    {
      perror("Failed to fork helper for remove_platform");
    }
  }

  log_event("Operator removed platform. New capacity: %d, Target drones: %d", new_cap, new_target);
}

void restock_drones(int sem_id, SharedStorage *shm)
{
  lock_sem(sem_id, SEM_SHM_ACCESS);
  int target = shm->base.target_drone_count;
  int current_total = shm->base.total_drones;
  int current_in_base = shm->base.current_drones;
  int max_in_base = shm->base.max_capacity;
  unlock_sem(sem_id, SEM_SHM_ACCESS);

  if (current_total < target)
  {
    int needed = target - current_total;
    int space = max_in_base - current_in_base;

    if (space > 0)
    {
      int to_spawn = (needed < space) ? needed : space;
      if (to_spawn > 0)
      {
        log_event("Operator restocking %d drones", to_spawn);
        spawn_drones(to_spawn);
      }
    }
  }
}

/**
 * @brief Runs the operator program, responsible for adding and removing platforms, and restocking drones.
 *
 * The operator program sets up a signal handler for SIG_ADD_PLATFORM and SIG_REMOVE_PLATFORM signals.
 * It then enters an infinite loop, where it waits for RESTOCK_DRONES_TIME seconds, checks if any signals have been received, and processes them.
 * If a ADD PLATFORM signal has been received, it adds a platform.
 * If a REMOVE PLATFORM signal has been received, it removes a platform.
 * It then restocks drones if necessary.
 */
void run_operator(void)
{
  setup_sigchld_handler();

  int shm_id = get_shm_id();
  SharedStorage *shm = attach_shm(shm_id);
  int sem_id = get_sem_id();

  // Register PID
  lock_sem(sem_id, SEM_SHM_ACCESS);
  shm->operator_pid = getpid();
  unlock_sem(sem_id, SEM_SHM_ACCESS);

  struct sigaction sa;
  sa.sa_handler = handle_op_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIG_ADD_PLATFORM, &sa, NULL);
  sigaction(SIG_REMOVE_PLATFORM, &sa, NULL);

  printf("Operator started. PID: %d\n", getpid());

  while (1)
  {
    sleep(RESTOCK_DRONES_TIME);

    if (add_platform_sig)
    {
      add_platform_sig = 0;
      add_platform(sem_id, shm);
    }

    if (remove_platform_sig)
    {
      remove_platform_sig = 0;
      remove_platform(sem_id, shm);
    }

    restock_drones(sem_id, shm);
  }
}

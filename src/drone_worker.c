#include "types.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

int drone_id;
int shm_id;
int sem_id;
SharedStorage *shm;
DroneInfo *my_info;

volatile sig_atomic_t pending_destruction = 0;

void handle_sigterm(int sig)
{
  (void)sig;
  exit(0);
}

void handle_attack_signal(int sig)
{
  if (sig == SIG_KILL)
  {
    if (my_info && my_info->battery < 20)
    {
      lock_sem(sem_id, SEM_SHM_ACCESS);
      snprintf(shm->last_notification, sizeof(shm->last_notification), "Drone %d ignored kill signal (Battery: %d%%)", drone_id, my_info->battery);
      shm->notification_time = time(NULL);
      unlock_sem(sem_id, SEM_SHM_ACCESS);
      log_event("Drone %d ignored kill signal (Low Battery)", drone_id);
    }
    else
    {
      pending_destruction = 1;
      log_event("Drone %d accepted kill signal", drone_id);
    }
  }
}

void clean_exit_after_delay()
{
  int was_in_base = 0;

  // 1. Mark as DESTROYED
  lock_sem(sem_id, SEM_SHM_ACCESS);
  if (my_info->state == IN_BASE)
    was_in_base = 1;

  my_info->state = DESTROYED;
  unlock_sem(sem_id, SEM_SHM_ACCESS);

  log_event("Drone %d destroyed", drone_id);

  // 2. Wait for 3 seconds
  sleep(3);

  // 3. Cleanup and exit
  lock_sem(sem_id, SEM_SHM_ACCESS);

  if (was_in_base)
  {
    shm->base.current_drones--;
  }

  shm->base.total_drones--;
  my_info->active = 0;

  unlock_sem(sem_id, SEM_SHM_ACCESS);

  exit(0);
}

void lock_sem_with_drain(int sem_num)
{
  while (1)
  {
    struct sembuf sb = {sem_num, -1, SEM_UNDO};
    struct timespec ts = {1, 0}; // 1 second timeout

    if (semtimedop(sem_id, &sb, 1, &ts) == 0)
    {
      return;
    }

    if (errno == EAGAIN)
    {
      // Drain battery
      my_info->battery -= 10;

      if (my_info->battery <= 0)
      {
        my_info->battery = 0;
        clean_exit_after_delay();
      }
    }
    else if (errno == EINTR)
    {
      if (pending_destruction)
        clean_exit_after_delay();
    }
    else
    {
      if (errno != EIDRM && errno != EINVAL)
      {
        perror("semtimedop");
      }
      exit(1);
    }

    if (pending_destruction)
      clean_exit_after_delay();
  }
}

void main_loop()
{
  if (pending_destruction)
    clean_exit_after_delay();

  // --- IN BASE ---
  if (my_info->state == IN_BASE)
  {
    // Charge
    sleep(CHARGE_TIME);
    if (pending_destruction)
      clean_exit_after_delay();

    my_info->battery = 100;

    log_event("Drone %d charged", drone_id);

    if (my_info->visits >= MAX_BASE_VISITS)
    {
      log_event("Drone %d retiring after %d visits", drone_id, my_info->visits);
      clean_exit_after_delay();
    }

    // Try to leave
    int passage = (rand() % 2 == 0) ? SEM_PASSAGE_1 : SEM_PASSAGE_2;

    lock_sem(sem_id, passage);
    if (pending_destruction)
    {
      unlock_sem(sem_id, passage);
      clean_exit_after_delay();
    }

    lock_sem(sem_id, SEM_SHM_ACCESS);
    my_info->state = IN_PASSAGE;
    shm->base.current_drones--;
    unlock_sem(sem_id, SEM_SHM_ACCESS);

    // Leave base capacity slot (Release P with UNDO)
    struct sembuf sb = {SEM_BASE_CAPACITY, 1, SEM_UNDO};
    semop(sem_id, &sb, 1);

    log_event("Drone %d leaving base", drone_id);

    sleep(1); // Time passage
    if (pending_destruction)
    {
      unlock_sem(sem_id, passage);
      clean_exit_after_delay();
    }

    unlock_sem(sem_id, passage);

    lock_sem(sem_id, SEM_SHM_ACCESS);
    my_info->state = IN_FLIGHT;
    unlock_sem(sem_id, SEM_SHM_ACCESS);
  }

  if (pending_destruction)
    clean_exit_after_delay();

  // --- IN FLIGHT ---
  if (my_info->state == IN_FLIGHT)
  {
    int flight_time = FLIGHT_TIME;
    // Fly
    for (int t = 0; t < flight_time; ++t)
    {
      if (pending_destruction)
        clean_exit_after_delay();
      sleep(1);
      my_info->battery -= 10;
      if (my_info->battery <= 20)
        break;
    }

    if (pending_destruction)
      clean_exit_after_delay();

    log_event("Drone %d returning to base (Battery: %d%%)", drone_id, my_info->battery);

    // Return
    lock_sem_with_drain(SEM_BASE_CAPACITY);
    if (pending_destruction)
    {
      struct sembuf sb_release = {SEM_BASE_CAPACITY, 1, SEM_UNDO};
      semop(sem_id, &sb_release, 1);
      clean_exit_after_delay();
    }

    int passage = (rand() % 2 == 0) ? SEM_PASSAGE_1 : SEM_PASSAGE_2;

    // Wait for Passage
    lock_sem_with_drain(passage);

    if (pending_destruction)
    {
      struct sembuf sb_release = {SEM_BASE_CAPACITY, 1, SEM_UNDO};
      semop(sem_id, &sb_release, 1);
      unlock_sem(sem_id, passage);
      clean_exit_after_delay();
    }

    lock_sem(sem_id, SEM_SHM_ACCESS);
    my_info->state = IN_PASSAGE;
    unlock_sem(sem_id, SEM_SHM_ACCESS);

    sleep(1); // Passage
    if (pending_destruction)
    {
      unlock_sem(sem_id, passage);
      clean_exit_after_delay();
    }

    lock_sem(sem_id, SEM_SHM_ACCESS);
    my_info->state = IN_BASE;
    my_info->visits++;
    shm->base.current_drones++;
    unlock_sem(sem_id, SEM_SHM_ACCESS);

    unlock_sem(sem_id, passage);
  }

  if (my_info->state == DESTROYED)
  {
    clean_exit_after_delay();
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    perror(
        "There are not enough arguments.\nWas the process spawned correctly?");
    return 1;
  }

  drone_id = atoi(argv[1]);

  shm_id = get_shm_id();
  sem_id = get_sem_id();
  shm = attach_shm(shm_id);
  my_info = &shm->drones[drone_id];

  srand(time(NULL) + getpid());

  log_event("Drone %d started", drone_id);

  signal(SIG_KILL, handle_attack_signal);
  signal(SIGTERM, handle_sigterm);

  while (1)
  {
    main_loop();
  }

  return 0;
}
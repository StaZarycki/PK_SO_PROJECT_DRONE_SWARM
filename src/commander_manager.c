#include "commander_manager.h"
#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void run_commander(int pipe_fd)
{
  int shm_id = get_shm_id();
  SharedStorage *shm = attach_shm(shm_id);
  int sem_id = get_sem_id();

  printf("Commander started. PID: %d. Waiting for commands...\n", getpid());

  char cmd;
  while (read(pipe_fd, &cmd, 1) > 0)
  {
    if (cmd == 'a') // Add Platform
    {
      if (shm->operator_pid > 0)
      {
        kill(shm->operator_pid, SIG_ADD_PLATFORM);
        log_event("Commander sent ADD PLATFORM signal");
      }
    }
    else if (cmd == 'r') // Remove Platform
    {
      if (shm->operator_pid > 0)
      {
        kill(shm->operator_pid, SIG_REMOVE_PLATFORM);
        log_event("Commander sent REMOVE PLATFORM signal");
      }
    }
    else if (cmd == 'k') // Kill Drone (Suicide Attack)
    {
      int target_id;
      if (read(pipe_fd, &target_id, sizeof(int)) > 0)
      {
        lock_sem(sem_id, SEM_SHM_ACCESS);
        if (target_id >= 0 && target_id < MAX_DRONES_TOTAL && shm->drones[target_id].active)
        {
          pid_t target_pid = shm->drones[target_id].pid;
          if (target_pid > 0)
          {
            kill(target_pid, SIG_KILL);
            log_event("Commander sent KILL signal to Drone %d", target_id);
          }
        }
        unlock_sem(sem_id, SEM_SHM_ACCESS);
      }
    }
  }

  close(pipe_fd);
  printf("Commander exiting.\n");
}
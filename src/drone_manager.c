#include "drone_manager.h"
#include "types.h"
#include "utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief Spawns a specified number of drones.
 * @param number The number of drones to spawn.
 * @return 0 on success, 1 on failure.
 *
 * This function spawns a specified number of drones. It iterates over the
 * available slots in the shared memory and marks them as active.
 * It then forks a new process for each drone and executes the drone_worker
 * program. The drone's PID is stored in the shared memory.
 *
 * If there are no available slots, the function returns 0. If there is an error
 * while forking or executing the drone_worker program, the function returns 1.
 */
int spawn_drones(int number)
{
  char drone_id_str[10];
  int shm_id = get_shm_id();
  SharedStorage *shm = attach_shm(shm_id);
  int sem_id = get_sem_id();

  for (int i = 0; i < number; i++)
  {
    struct sembuf sb = {SEM_BASE_CAPACITY, -1, IPC_NOWAIT};
    int initial_state = IN_FLIGHT;
    if (semop(sem_id, &sb, 1) == 0)
    {
      initial_state = IN_BASE;
    }
    else if (errno != EAGAIN)
    {
      perror("semop failed in spawn_drones");
    }

    lock_sem(sem_id, SEM_SHM_ACCESS);
    int slot = -1;
    for (int j = 0; j < MAX_DRONES_TOTAL; ++j)
    {
      if (!shm->drones[j].active)
      {
        slot = j;
        shm->drones[j].active = 1;
        shm->drones[j].id = j;
        shm->drones[j].visits = 0;
        shm->drones[j].battery = 100;
        shm->drones[j].state = initial_state;
        break;
      }
    }
    unlock_sem(sem_id, SEM_SHM_ACCESS);

    if (slot == -1)
    {
      if (initial_state == IN_BASE)
      {
        sb.sem_op = 1;
        sb.sem_flg = 0;
        semop(sem_id, &sb, 1);
      }
      return 0;
    }

    pid_t pid = fork();

    if (pid < 0) // Error
    {
      perror("Could not spawn a drone.\n");

      // Cleanup slot
      lock_sem(sem_id, SEM_SHM_ACCESS);
      shm->drones[slot].active = 0;
      unlock_sem(sem_id, SEM_SHM_ACCESS);

      if (initial_state == IN_BASE)
      {
        sb.sem_op = 1;
        sb.sem_flg = 0;
        semop(sem_id, &sb, 1);
      }

      return 1;
    }
    else if (pid == 0) // Child process
    {
      sprintf(drone_id_str, "%d", slot);

      execl("build/bin/drone_worker", "drone_worker", drone_id_str, NULL);
      execl("./bin/drone_worker", "drone_worker", drone_id_str, NULL);
      execl("./drone_worker", "drone_worker", drone_id_str, NULL);

      perror("Could not exec drone_worker.");
      exit(0);
    }
    else // Parent process
    {
      lock_sem(sem_id, SEM_SHM_ACCESS);
      shm->drones[slot].pid = pid;
      if (initial_state == IN_BASE)
      {
        shm->base.current_drones++;
      }
      shm->base.total_drones++;
      unlock_sem(sem_id, SEM_SHM_ACCESS);
    }
  }

  return 0;
}

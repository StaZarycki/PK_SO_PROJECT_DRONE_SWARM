#include "drone_manager.h"
#include "types.h"
#include "utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int spawn_drones(int number)
{
  char drone_id_str[10];
  int shm_id = get_shm_id();
  SharedStorage *shm = attach_shm(shm_id);
  int sem_id = get_sem_id();

  for (int i = 0; i < number; i++)
  {
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
        shm->drones[j].state = IN_BASE;
        break;
      }
    }
    unlock_sem(sem_id, SEM_SHM_ACCESS);

    if (slot == -1)
    {
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
      shm->base.current_drones++;
      shm->base.total_drones++;
      unlock_sem(sem_id, SEM_SHM_ACCESS);
    }
  }

  return 0;
}

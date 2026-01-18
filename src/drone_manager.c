#include "drone_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_DRONE_AMOUNT 150

pid_t drone_pids[MAX_DRONE_AMOUNT];

int spawn_drones(int number)
{
  char drone_id_str[4];

  if (number < 1)
  {
    perror("The number of drones must be at least 1.");
    return 1;
  }

  if (number > MAX_DRONE_AMOUNT)
  {
    fprintf(stderr, "The number of drones must be less than %d.",
            MAX_DRONE_AMOUNT);
  }

  for (int i = 0; i < number; i++)
  {
    pid_t pid = fork();

    if (pid < 0)
    {
      perror("Could not spawn a drone.\n");
      exit(1);
    }
    else if (pid == 0)
    {
      sprintf(drone_id_str, "%d", i);
      execl("./drone_worker", "drone_worker", drone_id_str, NULL);

      perror("Could not exec drone_worker.");
      exit(0);
    }
    else
    {
      drone_pids[i] = pid;
      printf("Created a drone: %d\n", pid);
    }
  }

  return 0;
}

#include "operator_manager.h"
#include "drone_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

Base base;

void handle_signal(int sig)
{
  if (sig == SIG_ADD_PLATFORM)
  {
    char buffer[100];

    int len = sprintf(buffer, "Adding a platform\n");
    write(STDOUT_FILENO, buffer, len);
    add_platform();
  }
  else if (sig == SIG_REMOVE_PLATFORM)
  {

    char buffer[100];

    int len = sprintf(buffer, "Removing a platform\n");
    write(STDOUT_FILENO, buffer, len);
    remove_platform();
  }
}

/**
 * Initializes the operator manager.
 *
 * Sets up signal handlers for SIG_ADD_PLATFORM and SIG_REMOVE_PLATFORM
 * and initializes the state of the operator manager.
 *
 * Returns 1 on failure to set up signal handlers, 0 on success.
 */
int init()
{
  base.max_drones = 75;
  base.current_drones = 0;
  base.platforms = 1;
  base.passages[0] = 0;
  base.passages[1] = 0;

  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;

  if (sigaction(SIG_ADD_PLATFORM, &sa, NULL) == -1)
  {
    perror("Could not set signal handler (SIG_ADD_PLATFORM)");
    return 1;
  }
  if (sigaction(SIG_REMOVE_PLATFORM, &sa, NULL) == -1)
  {
    perror("Could not set signal handler (SIG_REMOVE_PLATFORM)");
    return 1;
  }

  printf("Operator initialized.\n");

  while (1)
  {
    sleep(1);
  }
}

/**
 * @brief Spawn drones to reach the maximum allowed number.
 *
 * Spawns drones until the maximum number of drones allowed by the
 * operator is reached. The number of drones spawned is the difference
 * between the maximum number of drones and the current number of drones.
 *
 * @return 0 on success.
 */
int add_drones(void)
{
  spawn_drones(base.max_drones - base.current_drones);

  return 0;
}

/**
 * @brief Increase the maximum number of drones that can be controlled by the operator.
 *
 * The maximum number of drones that can be controlled by the operator is
 * doubled.
 *
 * @return 0 on success.
 */
int add_platform(void)
{
  base.max_drones *= 2;

  return 0;
}

/**
 * @brief Decrease the maximum number of drones that can be controlled by the operator.
 *
 * The maximum number of drones that can be controlled by the operator is
 * halved.
 *
 * @return 0 on success.
 */
int remove_platform(void)
{
  base.max_drones /= 2;

  return 0;
}

/**
 * @brief Get the status of a passage.
 *
 * @param n 0 or 1, corresponding to the first or second passage.
 * @return 0 if the passage is empty, 1 if it is occupied.
 */
int check_passage(int n)
{
  return base.passages[n];
}

#include "drone_worker.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int drone_id, battery_level, base_visits;
DroneState current_state;

void handle_attack_signal(int sig)
{
  if (sig == SIG_KILL)
  {
    char buffer[100];

    if (battery_level < 20)
    {
      int len = sprintf(buffer, "Drone %d is ignoring the signal (battery level is too low)\n", drone_id);
      write(STDOUT_FILENO, buffer, len);
    }
    else
    {
      int len = sprintf(buffer, "Drone %d is being destroyed\n", drone_id);
      write(STDOUT_FILENO, buffer, len);
      destroy();
    }
  }
}

void main_loop()
{
  sleep(1);

  drain_battery();
  printf("Drone number %d is working.\n", drone_id);
  printf("Battery level: %d\n", battery_level);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    perror(
        "There are not enough arguments.\nWas the process spawned correctly?");
    return 1;
  }

  struct sigaction sa;
  sa.sa_handler = handle_attack_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;

  if (sigaction(SIG_KILL, &sa, NULL) == -1)
  {
    perror("Could not set signal handler");
    return 1;
  }

  drone_id = atoi(argv[1]);
  battery_level = 100;
  current_state = IN_BASE;
  base_visits = 0;

  printf("Spawned drone number %d!\n", drone_id);

  while (1)
  {
    main_loop();
  }

  return 0;
}

/**
 * @brief Immediately terminates the drone worker process.
 *
 * This function will cause the immediate termination of the
 * drone worker process. It should be called when the drone
 * worker should shut down.
 */
void destroy(void)
{
  printf("Drone number %d is being destroyed\n", drone_id);

  exit(0);
}

/**
 * @brief Set the current state of the drone.
 *
 * @param state The state to set the drone to.
 *
 * Sets the current state of the drone to the given state.
 */
int set_state(DroneState state)
{
  current_state = state;

  if (state == IN_BASE)
  {
    if (base_visits++ > MAX_BASE_VISITS)
    {
      destroy();
    }
  }

  return 0;
}

/**
 * @brief Drain the drone's battery by 10 percentage points.
 *
 * Decreases the drone's battery level by 10 percentage points. If the
 * battery level drops to 0, the drone is immediately shut down. If
 * the battery level drops to 20 or below, the drone is set to IN_PASSAGE
 * state.
 */
int drain_battery(void)
{
  battery_level -= 10;

  if (battery_level <= 0)
  {
    destroy();
  }
  else if (battery_level <= 20)
  {
    set_state(IN_PASSAGE);
  }

  return 0;
}

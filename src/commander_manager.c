#include "commander_manager.h"
#include "types.h"

#include <signal.h>
#include <stdio.h>

/**
 * Send a kill signal to a drone process with the given pid.
 *
 * @param pid The process ID of the process to kill.
 *
 * @return 0 on success, or -1 on failure with errno set accordingly.
 */
int call_attack_drone(pid_t pid)
{
  if (kill(pid, SIG_KILL) == 0)
  {
    printf("Sent a kill signal to pid %d\n", pid);
  }
  else
  {
    perror("Could not send a kill signal to pid");
  }

  return 0;
}

/**
 * Send a signal to an operator process to add a platform.
 *
 * @param pid The process ID of the process to send the signal to.
 *
 * @return 0 on success, or -1 on failure with errno set accordingly.
 */
int call_add_platform(pid_t pid)
{
  if (kill(pid, SIG_ADD_PLATFORM) == 0)
  {
    printf("Sent an add platform signal to pid %d\n", pid);
  }
  else
  {
    perror("Could not send an add platform signal to pid");
  }

  return 0;
}

/**
 * Send a signal to an operator process to remove a platform.
 *
 * @param pid The process ID of the process to send the signal to.
 *
 * @return 0 on success, or -1 on failure with errno set accordingly.
 */
int call_remove_platform(pid_t pid)
{
  if (kill(pid, SIG_REMOVE_PLATFORM) == 0)
  {
    printf("Sent a remove platform signal to pid %d\n", pid);
  }
  else
  {
    perror("Could not send a remove platform signal to pid");
  }

  return 0;
}

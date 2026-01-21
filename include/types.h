#pragma once

#include <signal.h>

#define SIG_ADD_PLATFORM (SIGRTMIN)        // SIGNAL 1
#define SIG_REMOVE_PLATFORM (SIGRTMIN + 1) // SIGNAL 2
#define SIG_KILL (SIGRTMIN + 2)            // SIGNAL 3

typedef enum DroneState
{
  IN_BASE,
  IN_PASSAGE,
  IN_FLIGHT
} DroneState;
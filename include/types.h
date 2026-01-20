#pragma once

#define SIG_ADD_PLATFORM 1
#define SIG_REMOVE_PLATFORM 2
#define SIG_KILL 3

typedef enum DroneState
{
  IN_BASE,
  IN_PASSAGE,
  IN_FLIGHT
} DroneState;
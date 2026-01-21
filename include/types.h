#pragma once

#include <signal.h>

#define SIG_ADD_PLATFORM (SIGRTMIN)        // SIGNAL 1
#define SIG_REMOVE_PLATFORM (SIGRTMIN + 1) // SIGNAL 2
#define SIG_KILL (SIGRTMIN + 2)            // SIGNAL 3

#define CHARGE_TIME 5                   // T1
#define FLIGHT_TIME (CHARGE_TIME * 2.5) // T2
#define RESTOCK_DRONES_TIME 10          // Tk
#define MAX_BASE_VISITS 5               // X

typedef enum DroneState
{
  IN_BASE,
  IN_PASSAGE,
  IN_FLIGHT
} DroneState;
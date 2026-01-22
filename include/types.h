#pragma once

#include <signal.h>

#define SIG_ADD_PLATFORM (SIGRTMIN)        // SIGNAL 1
#define SIG_REMOVE_PLATFORM (SIGRTMIN + 1) // SIGNAL 2
#define SIG_KILL (SIGRTMIN + 2)            // SIGNAL 3

#define CHARGE_TIME 5                   // T1
#define FLIGHT_TIME (CHARGE_TIME * 2.5) // T2
#define RESTOCK_DRONES_TIME 10          // Tk
#define MAX_BASE_VISITS 5               // X

#define MAX_DRONES_TOTAL 1000

#define FTOK_PATH "CMakeLists.txt"
#define FTOK_PROJ_ID 1

#define SEM_BASE_CAPACITY 0
#define SEM_PASSAGE_1 1
#define SEM_PASSAGE_2 2
#define SEM_SHM_ACCESS 3
#define SEM_COUNT 4

typedef enum DroneState
{
  IN_BASE,
  IN_PASSAGE,
  IN_FLIGHT,
  DESTROYED
} DroneState;

typedef struct DroneInfo
{
  pid_t pid;
  int id;
  int battery;
  int visits;
  int active; // 1 if alive, 0 if destroyed
  DroneState state;
} DroneInfo;

typedef struct BaseState
{
  int current_drones;
  int max_capacity;        // P
  int total_drones;        // Currently active
  int target_drone_count;  // Dynamic N
  int initial_drone_count; // Constant N
} BaseState;

typedef struct SharedStorage
{
  pid_t operator_pid;
  BaseState base;
  char last_notification[128];
  time_t notification_time;
  DroneInfo drones[MAX_DRONES_TOTAL];
} SharedStorage;

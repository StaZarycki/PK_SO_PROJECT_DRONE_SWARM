#pragma once

#include "types.h"
#include <sys/types.h>

int spawn_drones(int number);
int destroy(pid_t pid);
int set_state(pid_t pid, DroneState state);
int drain_battery(pid_t pid);

#pragma once

#include <sys/types.h>

int call_attack_drone(pid_t pid);
int call_add_platform(pid_t pid);
int call_remove_platform(pid_t pid);
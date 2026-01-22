#pragma once

#include "types.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int get_shm_id(void);
int get_sem_id(void);
void lock_sem(int sem_id, int sem_num);
void unlock_sem(int sem_id, int sem_num);
void setup_sigchld_handler(void);
SharedStorage *attach_shm(int shm_id);

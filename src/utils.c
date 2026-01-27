#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <time.h>

void sigchld_handler(int sig)
{
  (void)sig;
  // Reap all dead children
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

int get_shm_id(void)
{
  key_t key = ftok(FTOK_PATH, FTOK_PROJ_ID);
  if (key == -1)
  {
    perror("ftok");
    exit(1);
  }

  return shmget(key, sizeof(SharedStorage), 0666 | IPC_CREAT);
}

int get_sem_id(void)
{
  key_t key = ftok(FTOK_PATH, FTOK_PROJ_ID);
  if (key == -1)
  {
    perror("ftok");
    exit(1);
  }

  return semget(key, SEM_COUNT, 0666 | IPC_CREAT);
}

void lock_sem(int sem_id, int sem_num)
{
  struct sembuf sb = {sem_num, -1, SEM_UNDO};
  if (semop(sem_id, &sb, 1) == -1)
  {
    if (errno == EIDRM || errno == EINVAL || errno == EINTR)
    {
      return;
    }

    perror("lock_sem");
    // exit(1);
  }
}

void unlock_sem(int sem_id, int sem_num)
{
  struct sembuf sb = {sem_num, 1, SEM_UNDO};
  if (semop(sem_id, &sb, 1) == -1)
  {
    if (errno == EIDRM || errno == EINVAL || errno == EINTR)
    {
      return;
    }

    perror("unlock_sem");
    exit(1);
  }
}

void setup_sigchld_handler(void)
{
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("sigaction");
    exit(1);
  }
}

SharedStorage *attach_shm(int shm_id)
{
  SharedStorage *shm = (SharedStorage *)shmat(shm_id, NULL, 0);
  if (shm == (void *)-1)
  {
    perror("shmat");
    exit(1);
  }

  return shm;
}

void log_event(const char *fmt, ...)
{
  int sem_id = get_sem_id();
  lock_sem(sem_id, SEM_LOG_ACCESS);

  FILE *fp = fopen("simulation.log", "a");
  if (fp)
  {
    time_t now = time(NULL);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(fp, "[%s] [PID:%d] ", time_buf, getpid());

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fprintf(fp, "\n");
    fclose(fp);
  }

  unlock_sem(sem_id, SEM_LOG_ACCESS);
}

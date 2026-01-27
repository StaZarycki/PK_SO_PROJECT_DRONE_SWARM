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

/**
 * @brief Get a shared memory segment.
 *
 * @return The ID of a shared memory segment, or -1 on error.
 *
 * This function uses ftok to generate a key based on the path
 * `FTOK_PATH` and the project ID `FTOK_PROJ_ID`. It then uses
 * shmget to allocate a shared memory segment of size
 * `sizeof(SharedStorage)` with permissions `0666` and the
 * `IPC_CREAT` flag. If either ftok or shmget fail, the function
 * prints an error message and exits with status 1.
 */
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

/**
 * @brief Get a semaphore set.
 *
 * @return The ID of a semaphore set, or -1 on error.
 *
 * This function uses ftok to generate a key based on the path
 * `FTOK_PATH` and the project ID `FTOK_PROJ_ID`. It then uses
 * semget to allocate a semaphore set of size `SEM_COUNT` with
 * permissions `0666` and the `IPC_CREAT` flag. If either ftok or
 * semget fail, the function prints an error message and exits with
 * status 1.
 */
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

/**
 * @brief Lock a semaphore.
 *
 * @param sem_id The ID of the semaphore set.
 * @param sem_num The number of the semaphore to lock.
 *
 * This function locks the semaphore with number `sem_num` in the
 * semaphore set identified by `sem_id`. If the locking fails, it
 * checks if the error was due to EIDRM, EINVAL, or EINTR and returns
 * immediately if so. Otherwise, it prints an error message and
 * exits with status 1.
 */
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
  }
}

/**
 * @brief Unlock a semaphore.
 *
 * @param sem_id The ID of the semaphore set.
 * @param sem_num The number of the semaphore to unlock.
 *
 * This function unlocks the semaphore with number `sem_num` in the
 * semaphore set identified by `sem_id`. If the unlocking fails, it
 * checks if the error was due to EIDRM, EINVAL, or EINTR and returns
 * immediately if so. Otherwise, it prints an error message and
 * exits with status 1.
 */
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

/**
 * @brief Sets up a signal handler for SIGCHLD.
 *
 * This function sets up a signal handler for SIGCHLD, which is called
 * when a child process terminates. The handler, sigchld_handler,
 * reaps all dead children.
 */
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

/**
 * @brief Attaches to a shared memory segment.
 *
 * This function attaches to a shared memory segment with the given ID.
 * If the attaching fails, it prints an error message and exits with status 1.
 *
 * @param shm_id The ID of the shared memory segment to attach to.
 *
 * @return A pointer to the attached shared memory segment, or NULL on error.
 */
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

/**
 * @brief Logs an event to the simulation log file.
 *
 * This function logs an event to the file "simulation.log". The event
 * is prefixed with a timestamp in the format "YYYY-MM-DD HH:MM:SS"
 * and the process ID of the calling process. The event is then
 * followed by a newline character.
 *
 * @param fmt The format string for the event. This string should
 * contain any desired arguments for the event.
 *
 * @param ... The arguments for the format string.
 */
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

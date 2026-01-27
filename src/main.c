#define TB_IMPL

#include "drone_manager.h"
#include "utils.h"
#include "termbox2.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_DRONE_AMOUNT 150

int shm_id;
int sem_id;
pid_t operator_pid, commander_pid;
int commander_pipe[2];

int init_semaphores(int drone_amount)
{
  int P = drone_amount / 2;
  if (P < 2)
    P = 2; // Minimal capacity

  semctl(sem_id, SEM_BASE_CAPACITY, SETVAL, P);
  semctl(sem_id, SEM_PASSAGE_1, SETVAL, PASSAGE_CAPACITY);
  semctl(sem_id, SEM_PASSAGE_2, SETVAL, PASSAGE_CAPACITY);
  semctl(sem_id, SEM_SHM_ACCESS, SETVAL, 1);

  return P;
}

void init_shm(SharedStorage *shm, int max_capacity, int drone_amount)
{
  shm->base.current_drones = 0;
  shm->base.max_capacity = max_capacity;
  shm->base.total_drones = 0;
  shm->base.target_drone_count = drone_amount;
  shm->base.initial_drone_count = drone_amount;

  for (int i = 0; i < MAX_DRONES_TOTAL; ++i)
  {
    shm->drones[i].active = 0;
  }
}

void init_pipe(void)
{
  if (pipe(commander_pipe) == -1)
  {
    perror("pipe");
  }
}

void spawn_processes(int drone_amount)
{
  // Spawn Commander
  commander_pid = fork();
  if (commander_pid == 0)
  {
    close(commander_pipe[1]);
    run_commander(commander_pipe[0]);

    exit(0);
  }
  close(commander_pipe[0]);

  // Spawn Operator
  operator_pid = fork();
  if (operator_pid == 0)
  {
    run_operator();

    exit(0);
  }

  // Spawn Drones
  spawn_drones(drone_amount);
}

int main(int argc, char *argv[])
{
  int drone_amount = 0;

  if (argc >= 2)
  {
    drone_amount = atoi(argv[1]);
    if (drone_amount == -1)
      return 1;
  }
  else
  {
    drone_amount = DEFAULT_DRONE_AMOUNT;
  }

  shm_id = get_shm_id();
  sem_id = get_sem_id();

  int P = init_semaphores(drone_amount);

  SharedStorage *shm = attach_shm(shm_id);
  init_shm(shm, P, drone_amount);
  init_pipe();

  spawn_processes(drone_amount);

  struct tb_event ev;
  tb_init();

  int y = 0;
  tb_printf(0, y++, TB_WHITE, TB_BLACK, "Drone amount: %d", drone_amount);
  tb_printf(0, y++, TB_WHITE, TB_BLACK, "Press any key to quit...");
  tb_present();
  tb_poll_event(&ev);
  tb_shutdown();

  return 0;
}

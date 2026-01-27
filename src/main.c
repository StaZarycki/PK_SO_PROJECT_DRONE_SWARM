#define TB_IMPL

#include "drone_manager.h"
#include "commander_manager.h"
#include "operator_manager.h"
#include "utils.h"
#include "termbox2.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void cleanup()
{
  SharedStorage *shm = attach_shm(shm_id);

  // 1. Kill Operator and Commander
  if (operator_pid > 0)
    kill(operator_pid, SIGTERM);
  if (commander_pid > 0)
    kill(commander_pid, SIGTERM);

  // 2. Kill Drones
  if (shm != (void *)-1)
  {
    for (int i = 0; i < MAX_DRONES_TOTAL; ++i)
    {
      if (shm->drones[i].active && shm->drones[i].pid > 0)
      {
        kill(shm->drones[i].pid, SIGTERM);
      }
    }
  }

  // 3. Wait for cleanup
  usleep(200000); // 200ms

  // 4. Remove IPC
  shmctl(shm_id, IPC_RMID, NULL);
  semctl(sem_id, 0, IPC_RMID);

  tb_shutdown();
}

void handle_sigint(int sig)
{
  (void)sig;
  cleanup();
  exit(0);
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

  setup_sigchld_handler();

  shm_id = get_shm_id();
  sem_id = get_sem_id();

  int P = init_semaphores(drone_amount);

  SharedStorage *shm = attach_shm(shm_id);
  init_shm(shm, P, drone_amount);
  init_pipe();

  spawn_processes(drone_amount);

  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigint);

  struct tb_event ev;
  tb_init();

  char input_buf[10] = {0};
  int input_len = 0;
  int input_mode = 0; // 0: normal, 1: inputting drone ID
  int scroll_offset = 0;

  // Render UI
  while (1)
  {
    tb_clear();
    int height = tb_height();

    int header_height = 3;
    int footer_height = 3;
    int list_height = height - header_height - footer_height;
    if (list_height < 0)
      list_height = 0;

    // Header
    tb_printf(0, 0, TB_WHITE, TB_BLACK, "Drone Swarm Simulation");

    tb_printf(0, 1, TB_WHITE, TB_BLACK, "Drones: %d / %d (Base Capacity: %d) | Target: %d",

              shm->base.current_drones, shm->base.total_drones, shm->base.max_capacity, shm->base.target_drone_count);

    // Drone List
    int visual_idx = 0;
    for (int i = 0; i < MAX_DRONES_TOTAL; ++i)
    {
      if (shm->drones[i].active)
      {
        if (visual_idx >= scroll_offset && visual_idx < scroll_offset + list_height)
        {
          int y = header_height + (visual_idx - scroll_offset);
          const char *state_str = "UNK";
          uint16_t color = TB_WHITE;
          switch (shm->drones[i].state)
          {
          case IN_BASE:
            state_str = "BASE";
            color = TB_GREEN;
            break;
          case IN_PASSAGE:
            state_str = "PASS";
            color = TB_YELLOW;
            break;
          case IN_FLIGHT:
            state_str = "FLY ";
            color = TB_CYAN;
            break;
          case DESTROYED:
            state_str = "DEAD";
            color = TB_RED;
            break;
          }
          tb_printf(0, y, color, TB_BLACK, "Drone %3d: Battery %3d%% [%s] Visits: %d", shm->drones[i].id, shm->drones[i].battery, state_str, shm->drones[i].visits);
        }
        visual_idx++;
      }
    }
    int total_active_drones = visual_idx;

    // Notification
    if (time(NULL) - shm->notification_time < 5)
    {

      tb_printf(0, height - 3, TB_YELLOW | TB_BOLD, TB_BLACK, "INFO: %s", shm->last_notification);
    }

    // Footer
    int footer_y = height - 2;
    if (input_mode == 1)
    {
      tb_printf(0, footer_y, TB_MAGENTA, TB_BLACK, "KILL DRONE ID: %s_", input_buf);
      tb_printf(0, footer_y + 1, TB_WHITE, TB_BLACK, "[Enter] Confirm  [Esc] Cancel");
    }
    else
    {
      tb_printf(0, footer_y, TB_WHITE, TB_BLACK, "Controls: [q] Quit  [a] Add Platform  [r] Remove Platform  [k] Kill");
      tb_printf(0, footer_y + 1, TB_WHITE, TB_BLACK, "Scroll: [Up/Down]  [PgUp/PgDn]");
    }

    tb_present();

    tb_peek_event(&ev, 500); // Update every 500ms
    if (ev.type == TB_EVENT_KEY)
    {
      if (input_mode == 1)
      {
        if (ev.key == TB_KEY_ESC)
        {
          input_mode = 0;
          input_len = 0;
          input_buf[0] = '\0';
        }
        else if (ev.key == TB_KEY_ENTER)
        {
          int target_id = atoi(input_buf);

          lock_sem(sem_id, SEM_SHM_ACCESS);
          int exists = (target_id >= 0 && target_id < MAX_DRONES_TOTAL && shm->drones[target_id].active);
          unlock_sem(sem_id, SEM_SHM_ACCESS);

          if (exists)
          {
            write(commander_pipe[1], "k", 1);
            write(commander_pipe[1], &target_id, sizeof(int));
          }
          input_mode = 0;
          input_len = 0;
          input_buf[0] = '\0';
        }
        else if (ev.ch >= '0' && ev.ch <= '9' && input_len < 9)
        {
          input_buf[input_len++] = (char)ev.ch;
          input_buf[input_len] = '\0';
        }
        else if (ev.key == TB_KEY_BACKSPACE || ev.key == TB_KEY_BACKSPACE2)
        {
          if (input_len > 0)
          {
            input_buf[--input_len] = '\0';
          }
        }
      }
      else
      {
        if (ev.ch == 'q')
        {
          break;
        }
        if (ev.ch == 'a')
        {
          write(commander_pipe[1], "a", 1);
        }
        if (ev.ch == 'r')
        {
          write(commander_pipe[1], "r", 1);
        }
        if (ev.ch == 'k')
        {
          input_mode = 1;
          input_len = 0;
          input_buf[0] = '\0';
        }

        // Scrolling
        if (ev.key == TB_KEY_ARROW_DOWN)
        {
          if (scroll_offset < total_active_drones - list_height)
          {
            scroll_offset++;
          }
        }
        if (ev.key == TB_KEY_ARROW_UP)
        {
          if (scroll_offset > 0)
          {
            scroll_offset--;
          }
        }
        if (ev.key == TB_KEY_PGDN)
        {
          scroll_offset += list_height;
          if (scroll_offset > total_active_drones - list_height)
          {
            scroll_offset = total_active_drones - list_height;
          }
          if (scroll_offset < 0)
            scroll_offset = 0;
        }
        if (ev.key == TB_KEY_PGUP)
        {
          scroll_offset -= list_height;
          if (scroll_offset < 0)
            scroll_offset = 0;
        }
      }
    }
  }

  cleanup();
  return 0;
}

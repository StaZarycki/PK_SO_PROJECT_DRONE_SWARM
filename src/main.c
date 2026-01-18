#define TB_IMPL

#include "drone_manager.h"
#include "utils.h"
#include "termbox2.h"
#include <stdio.h>
#include <stdlib.h>

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
    drone_amount = 150;
  }

  spawn_drones(drone_amount);

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

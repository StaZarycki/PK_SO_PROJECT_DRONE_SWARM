#include "drone_manager.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int drone_amount = 0;

  if (argc >= 2) {
    drone_amount = atoi(argv[1]);
    if (drone_amount == -1)
      return 1;
  } else {
    drone_amount = 150;
  }

  printf("Drone amount: %d\n", drone_amount);

  spawn_drones(drone_amount);

  return 0;
}

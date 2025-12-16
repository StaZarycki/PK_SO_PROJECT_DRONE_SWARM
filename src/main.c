#include "utils.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  int drone_amount = 0;

  if (argc >= 2) {
    drone_amount = parse_int(argv[1]);
    if (drone_amount == -1)
      return 1;
  }

  printf("Drone amount: %d\n", drone_amount);

  return 0;
}

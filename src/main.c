#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int drone_amount = 0;

  if (argc >= 2) {
    char *response;
    drone_amount = strtol(argv[1], &response, 10);

    if (strcmp(response, ""))
      printf("Could not parse %s to int.\n", response);

    return 1;
  }

  printf("Drone amount: %d\n", drone_amount);

  return 0;
}

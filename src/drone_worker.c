#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    perror(
        "There are not enough arguments.\nWas the process spawned correctly?");
    return 1;
  }

  int drone_id = atoi(argv[1]);

  printf("Test from drone number %d!\n", drone_id);

  sleep(10);

  printf("Drone number %d is going to die!\n", drone_id);
}

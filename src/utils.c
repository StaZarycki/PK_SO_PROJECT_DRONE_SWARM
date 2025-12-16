#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_int(char *num_char) {
  int return_number = 0;
  char *response;
  return_number = strtol(num_char, &response, 10);

  if (strcmp(response, "")) {
    printf("Could not parse %s to int.\n", response);

    return -1;
  }

  return return_number;
}

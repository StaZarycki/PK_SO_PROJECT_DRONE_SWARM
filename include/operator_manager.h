#pragma once

typedef struct Base
{
  int max_drones;
  int current_drones;
  int platforms;
  int passages[2];
} Base;

int add_drones(void);
int add_platform(void);
int remove_platform(void);
int check_passage(int n);
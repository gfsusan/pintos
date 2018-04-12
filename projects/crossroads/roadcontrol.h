#include "position.h"

#define MAP_SIZE 7

struct crossroads {
  struct semaphore spot[MAP_SIZE][MAP_SIZE];
};

struct vehicle {
  char name;
  int src;
  int dest;
  int pos;
  struct crossroads *cr;
};

void crossroads_init(struct crossroads *cr);
void vehicle_init(struct vehicle * car, char name, const int src, const int dest, void * aux);

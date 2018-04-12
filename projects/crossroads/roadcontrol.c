#include "../threads/synch.h"
#include "../threads/interrupt.h"
#include "../threads/thread.h"

#include "roadcontrol.h"

#include <stdio.h>
#include <string.h>


void crossroads_init(struct crossroads *cr) {
  int i, j;
  for (i = 0; i < MAP_SIZE; i++) {
    for (j = 0; j < MAP_SIZE; j++) {
      if ( i == 2 || i == 3 || i == 4) {
        sema_init(&cr->spot[i][j], 1);
      } else if (i > 4 && (j>1 && j<5)) {
        sema_init(&cr->spot[i][j], 1);
      } else {
        sema_init(&cr->spot[i][j], 0);
      }
    }
  }

  // initialize position

}

void vehicle_init(struct vehicle * car, char name, const int src, const int dest, void * aux) {
  car->name = name;
  car->src = src;
  car->dest = dest;
  car->pos = -1;            // make sure not to access index -1 of an array
  car->cr = (struct crossroads *)aux;

}

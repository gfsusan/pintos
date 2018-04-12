#include <stdio.h>
#include <string.h>

#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

#include "devices/timer.h"
#include "devices/vga.h"

#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/mapdata.h"
#include "projects/crossroads/roadcontrol.h"

#define MAP_SIZE 7

int argc(const char* argv);
void hello_thread2(void* aux);
// parse argv and create threads
void create_vehicles(struct vehicle *cars, char *arg, void* aux);
void create_threads(struct vehicle *cars, struct crossroads *cr);

void move_vehicle(void* aux);
bool isCrossroad(struct position pos);

void run_crossroads(char **argv)
{
	int nVehicle = 0;
	struct crossroads cr;
	struct vehicle *cars;

	// initialize crossroads
	crossroads_init(&cr);
	nVehicle = argc(argv[1]);
	printf("%d", nVehicle);
	cars = (struct vehicle *)malloc(sizeof(struct vehicle) * nVehicle);

	// parse arguments
	// parse data ----- ex) aAB to a, A, B
	// create vehicles
	create_vehicles(cars, argv[1], &cr);

	// create threads
	for (int i = 0; i < nVehicle; i++ ) {
		char tempName[2];
		tempName[0] = cars[i].name;
		thread_create(tempName, PRI_DEFAULT, move_vehicle, &cars[i]);
	}

	// while (i++ != 20) {
	// 	timer_sleep (TIMER_FREQ);
	// 	printf("\033[2J");
		// move cursor to 0,0
		for (int i = 0; i < MAP_SIZE; i++) {
			for (int j = 0; j < MAP_SIZE; j++) {
				printf("%c ", map_draw_default[i][j]);
			}
			printf("\n");
		}
	// }

  thread_sleep(100);
	free(cars);
}

void hello_thread2(void* aux) {
	struct crossroads *cr = (struct crossroads *)aux;
	struct vehicle car;
}

void create_vehicles(struct vehicle *cars, char *arg, void* aux) {
	// struct crossroads *cr = (struct crossroads *)aux;
	char *token, *save_ptr;
	int i = 0;
	for (token = strtok_r (arg, ":", &save_ptr); token != NULL;
			 token = strtok_r (NULL, ":", &save_ptr)) {
		printf ("token: %s\n", token);
		char name, src, des;
		name = *(token);
		src = *(token + 1);
		des = *(token + 2);

		vehicle_init(&cars[i], name, src-'A', des-'B', aux);
		i++;
	}
}


int argc(const char* argv) {
	char* argument = argv;
	char c;
	int i = 0;
	int counter = 1;
	int length = strnlen(argv, 100);
	while (i++ < length) {
		c = *(argv + i);
		if(c == ':')
			counter++;
	}
	return counter;
}

void move_vehicle(void* aux) {
	struct vehicle *car = (struct vehicle *)aux;

}

bool isCrossroad(struct position pos) {
	for (int i = 0; i < 8; i++) {
		if (pos.row == cross[i].row)
			if (pos.col == cross[i].col)
				return true;
		return false;
	}
}

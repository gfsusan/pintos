#include <stdio.h>
#include <string.h>

#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"

#include "projects/memalloc/memalloctest.h"

void run_memalloc_test(char **argv UNUSED)
{
	size_t i;
	char* dynamicmem[11];

	// memset : 0 - 6
	for (i=0; i<7; i++) {
		dynamicmem[i] = (char *) malloc (145000);
		if (dynamicmem[i] != NULL) {
			memset(dynamicmem[i], 0x00, 145000);
		}
	}
	printf("dump page status : \n");
	palloc_get_status(0);
	thread_sleep(100);

	// free : 0 - 2
	for (i = 0; i<3; i++) {
		if (dynamicmem[i] != NULL) {
			free(dynamicmem[i]);
		}
		printf("dump page status : \n");
		palloc_get_status(0);
	}

	// memset : 7 - 9
	for (i = 7; i<10; i++) {
		dynamicmem[i] = (char *)malloc(145000);
		if (dynamicmem[i] != NULL) {
			memset(dynamicmem[i], 0x00, 145000);
			printf("dump page status : \n");
			palloc_get_status(0);
		}
	}

	//memset : 10
	dynamicmem[10] = (char *)malloc(16000);
	memset(dynamicmem[10], 0x00, 16000);
	printf("dump page status : \n");
	palloc_get_status(0);

	thread_sleep(100);

	// free
	for (i = 3; i<11; i++) {
		if (dynamicmem[i] != NULL) {
			free(dynamicmem[i]);
		}
		printf("dump page status : \n");
		palloc_get_status(0);
	}
}
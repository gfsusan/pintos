#include <stdio.h>
#include <string.h>

#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"

#include "projects/memalloc/memalloctest.h"

void run_memalloc_test(char **argv UNUSED)
{
	size_t k = 1024;
	size_t i = 0;
	char* dynamicmem[11];
	
	// memset : 0 - 3
	dynamicmem[0] = (char *) malloc (50 * k);
	if (dynamicmem[0] != NULL) {
		memset(dynamicmem[0], 0x00, (50 * k));
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	dynamicmem[1] = (char *) malloc (120 * k);
	if (dynamicmem[1] != NULL){
		memset(dynamicmem[1], 0x00, (120 * k));
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	dynamicmem[2] = (char *) malloc (32 * k);
	if (dynamicmem[2] != NULL) {
		memset(dynamicmem[2], 0x00, (32 * k));
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	dynamicmem[3] = (char *) malloc (128 * k);
	if (dynamicmem[3] != NULL) {
		memset(dynamicmem[3], 0x00, (128 * k));
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	// release : 1
	if (dynamicmem[1] != NULL) {
		free(dynamicmem[1]);
		printf("dump page status: \n");
		palloc_get_status(0);
	}
	
	// release : 0
	if (dynamicmem[0] != NULL) {
		free(dynamicmem[0]);
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	// memset : 4
	dynamicmem[4] = (char *) malloc (37 * k);
	if (dynamicmem[4] != NULL) {
		memset(dynamicmem[4], 0x00, (37 * k));
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	// release : 2
	if (dynamicmem[2] != NULL) {
		free(dynamicmem[2]);
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	// release : 4
	if (dynamicmem[4] != NULL) {
		free(dynamicmem[4]);
		printf("dump page status: \n");
		palloc_get_status(0);

	}

	// release : 3
	if (dynamicmem[3] != NULL) {
		free(dynamicmem[3]);
		printf("dump page status: \n");
		palloc_get_status(0);
	}

	thread_sleep(100);
}

#include <stdio.h>
#include <string.h>

#include "projects/msgpassing/msgpassingtest.h"
#include "../threads/thread.h"
#include "../threads/synch.h"

void receiver_thread(void* aux);
void receiver_thread2(void *aux);
void receiver_thread3(void* aux);
void receiver_thread4(void* aux);
void receiver_thread5(void* aux);

void sender_thread(void* aux);
void sender_thread2(void *aux);
void sender_thread3(void* aux);
void sender_thread4(void* aux);
void sender_thread5(void* aux);

void run_message_passing_test(void)
{
	struct messageBox box;


	printf("\n\n\n\n-------------------- START TESTING --------------------\n\n");

	messageBox_init(&box);

	thread_create("Sender 1", PRI_DEFAULT, sender_thread, &box);
	thread_sleep(100);
	
	thread_create("Sender 5", PRI_DEFAULT, sender_thread5, &box);
	thread_sleep(100);

	thread_create("Receiver 1", PRI_DEFAULT, receiver_thread, &box);
	thread_sleep(100);

	thread_create("Receiver 5", PRI_DEFAULT, receiver_thread5, &box);
	thread_sleep(100);

	thread_create("Sender 3", PRI_DEFAULT, sender_thread3, &box);
	thread_sleep(100);

	thread_create("Receiver 2", PRI_DEFAULT, receiver_thread2, &box);
	thread_sleep(100);

	thread_create("Receiver 3", PRI_DEFAULT, receiver_thread3, &box);
	thread_sleep(100);

	thread_create("Sender 2", PRI_DEFAULT, sender_thread2, &box);
	thread_sleep(100);

	thread_create("Sender 4", PRI_DEFAULT, sender_thread4, &box);
	thread_sleep(100);

	thread_create("Receiver 4", PRI_DEFAULT, receiver_thread4, &box);
	thread_sleep(100);


}

void hello_thread(void * aux) {
	struct messageBox * box = (struct messageBox *)aux;
	printf("%s\n\n\n", box->message);
}

void sender_thread(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30] = "I sent this first!";

	blocking_send(box, message);

	printf("Sender1 is now ready\n");
}

void sender_thread2(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30] = "I sent this second!";

	blocking_send(box, message);

	printf("Sender2 is now ready\n");
}

void sender_thread3(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30] = "I sent this third!";

	blocking_send(box, message);

	printf("Sender3 is now ready\n");
}

void sender_thread4(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30] = "I sent this fourth!";

	blocking_send(box, message);

	printf("Sender4 is now ready\n");
}

void sender_thread5(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30] = "I sent this fifth!";

	blocking_send(box, message);

	printf("Sender5 is now ready\n");
}

void receiver_thread(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30];

	memset(message, 0x00, 30);
	blocking_receive(box, message);

	printf("Receiver1 is now ready\n");
}

void receiver_thread2(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30];

	memset(message, 0x00, 30);
	blocking_receive(box, message);

	printf("Receiver2 is now ready\n");
}

void receiver_thread3(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30];

	memset(message, 0x00, 30);
	blocking_receive(box, message);

	printf("Receiver3 is now ready\n");
}

void receiver_thread4(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30];

	memset(message, 0x00, 30);
	blocking_receive(box, message);

	printf("Receiver4 is now ready\n");
}

void receiver_thread5(void *aux) {
	struct messageBox * box = (struct messageBox *)aux;
	char message[30];

	memset(message, 0x00, 30);
	blocking_receive(box, message);

	printf("Receiver5 is now ready\n");
}

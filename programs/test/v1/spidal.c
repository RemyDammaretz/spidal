#include "../includes/gpioLib.h"
#include "../includes/socketLib.h"
#include "../includes/check.h"
#include "../includes/appStatus.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define PIN 16

#define SERVER_PORT 1234

struct timespec time1;
struct timespec timeMess;
struct timespec deltaT;

status_t status;;
int sd;

void inputCallback(int gpio, int level, uint32_t tick);
void * messagesThread();

int main(void) {
	int se;
	pthread_t threadMessages;
	struct sockaddr_in serverAddr;
	
	initStatus(&status);
	
	printf("Init GPIO\n");
	initGPIO();
	setupInput(PIN, RISING_MODE, PULL_UP, inputCallback);
	
	printf("Waiting for connection...\n");
	serverConnect(&se, &sd, &serverAddr, SERVER_PORT);
	printf("Connected !\n");
	
	
	setStatus(&status, STATUS_WAITING_RACE);
	
	// Creating waiting message thread
	CHECK_T(
		pthread_create(&threadMessages, NULL, messagesThread, NULL),
		"--pthread_create()"
	);
	
	while(pause());

	CHECK_T(pthread_join(threadMessages, NULL), "--pthread_join()");
	
	close(sd);
	close(se);
	return 0;
}

void inputCallback(int gpio, int level, uint32_t tick) {
	if (getStatus(&status) == STATUS_WAITING_RACE) {
		clock_gettime(CLOCK_REALTIME, &time1);
		printf("Race started !\n");
		setStatus(&status, STATUS_WAITING_END_RACE);
	}
	
}

void * messagesThread() {
	while (1) {
		waitForMessage(sd, &timeMess, sizeof(timeMess));
		if (getStatus(&status) == STATUS_WAITING_END_RACE) {
			printf("End race !\n");
			//printf("Time button : %ld s %ld ns \n", time1.tv_sec, time1.tv_nsec);
			//printf("Time mess   : %ld s %ld ns \n", timeMess.tv_sec, timeMess.tv_nsec);
			deltaT.tv_sec = timeMess.tv_sec - time1.tv_sec;
			deltaT.tv_nsec = timeMess.tv_nsec - time1.tv_nsec;
			
			printf("Time  : %ld s %ld ns \n", deltaT.tv_sec, deltaT.tv_nsec);
			setStatus(&status, STATUS_WAITING_RACE);
		}
	}
	pthread_exit(NULL);
}


#include "../../include/gpioLib.h"
#include "../../include/socketLib.h"
#include "../../include/check.h"
#include "../../include/appStatus.h"
#include "../../include/signalsLib.h"
#include "../../include/messages.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define PIN1 16
#define PIN2 13

#define BUTTON_MODE_INV 0

#define SERVER_PORT 1234

struct timespec timeInterrupt;
struct timespec time0;

struct timespec timefP1;
struct timespec timefP2;

raceMessage message;

status_t status;
int sd;

double timeResult1;
double timeResult2;

int endP1;
int endP2;

void inputCallback(int gpio, int level, uint32_t tick);
void signalsHandler(int sigNum);
void * messagesThread();
double computeTime(struct timespec ti, struct timespec tf);
void startRace();

int main(void) {
	int se;
	pthread_t threadMessages;
	struct sockaddr_in serverAddr;
	
	// Init status
	initStatus(&status, STATUS_START);
	
	// Init GPIO
	printf("Init GPIO\n");
	initGPIO();
	setupGPIOInput(PIN1, RISING_MODE, PULL_UP, inputCallback);
	setupGPIOInput(PIN2, RISING_MODE, PULL_UP, inputCallback);
	
	// Install signals handler
	installSignalHandler(SIGALRM, signalsHandler);
	
	// Connexion to spidalremote
	printf("Waiting for connection...\n");
	serverConnect(&se, &sd, &serverAddr, SERVER_PORT);
	printf("Connected !\n");	
	
	setStatus(&status, STATUS_WAITING);
	
	// Creating waiting message thread
	CHECK_T(
		pthread_create(&threadMessages, NULL, messagesThread, NULL),
		"--pthread_create()"
	);

	CHECK_T(pthread_join(threadMessages, NULL), "--pthread_join()");
	
	close(sd);
	close(se);
	return 0;
}

// Manage interuptor
void inputCallback(int gpio, int level, uint32_t tick) {
	clock_gettime(CLOCK_REALTIME, &timeInterrupt);
	switch (getStatus(&status)) {
		case STATUS_WAITING:
			// Setup alarm to launch a race in 1s if every runner is ready
			alarm(1);
			break;
		case STATUS_READY:
			// False start
			setStatus(&status, STATUS_WAITING);
			printf("False start for runner %d\n", gpio == PIN1 ? 1 : 2);
			break;
	}
}

// Listening to messages thread
void * messagesThread() {
	while (1) {
		waitForMessage(sd, &message, sizeof(message));
		if (getStatus(&status) == STATUS_RACE) {
			if (!endP1 && message.pId == 1) {
				endP1 = 1;
				timefP1 = message.time;
				
			} else if (!endP2 && message.pId == 2) {
				endP2 = 1;
				timefP2 = message.time;
			}
			
			if (endP1 && endP2) {
				printf("End race !\n");
				
				timeResult1 = computeTime(time0, timefP1);
				timeResult2 = computeTime(time0, timefP2);
				
				printf("Runner 1  : %.2lf s\n", timeResult1);
				printf("Runner 2  : %.2lf s\n", timeResult2);
				setStatus(&status, STATUS_WAITING);
			}
		}
	}
	pthread_exit(NULL);
}

// Manage signals
void signalsHandler(int sigNum) {
	switch(sigNum) {
		case SIGALRM:
			// Check if all pedals are pressed and if we are waiting to start a race
			#if BUTTON_MODE_INV
			if (getStatus(&status) == STATUS_WAITING && !getGPIOValue(PIN1) && !getGPIOValue(PIN2)) {
			#else
			if (getStatus(&status) == STATUS_WAITING && getGPIOValue(PIN1) && getGPIOValue(PIN2)) {			
			#endif
				startRace();
			}
			break;
	}
}

double computeTime(struct timespec ti, struct timespec tf) {
	double result;
	int t_s = tf.tv_sec - ti.tv_sec;
	int t_ns = tf.tv_nsec - ti.tv_nsec;
	if (t_ns < 0) {
		t_s--;
		t_ns = (1000000000 + t_ns); // Rounding number at the same time
	}
	result = t_s + t_ns/1000000000.0;
	return result;
	
}

void startRace() {
	// Start race
	int i;
	
	setStatus(&status, STATUS_READY);
	endP1 = 0; endP2 = 0;
	printf("Ready...\n");
	sleep(1);
	
	for (i = 3; i > 0; i--) {
		if (getStatus(&status) != STATUS_READY) return; // In case of false start
		printf("%d\n", i);
		sleep(1);
	}
	
	if (getStatus(&status) != STATUS_READY) return; // In case of false start
	clock_gettime(CLOCK_REALTIME, &time0);
	setStatus(&status, STATUS_RACE);
	printf("Go !\n");
}


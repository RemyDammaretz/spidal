#include "../../include/gpioLib.h"
#include "../../include/socketLib.h"
#include "../../include/check.h"
#include "../../include/appStatus.h"
#include "../../include/signalsLib.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define PIN 16
#define BUTTON_MODE_INV 0

#define SERVER_PORT 1234

struct timespec timeInterrupt;
struct timespec time0;
struct timespec timeMess;

status_t status;;
int sd;

void inputCallback(int gpio, int level, uint32_t tick);
void signalsHandler(int sigNum);
void * messagesThread();
double computeTime(struct timespec ti, struct timespec tf);

int main(void) {
	int se;
	pthread_t threadMessages;
	struct sockaddr_in serverAddr;
	
	// Init status
	initStatus(&status, STATUS_START);
	
	// Init GPIO
	printf("Init GPIO\n");
	initGPIO();
	setupGPIOInput(PIN, RISING_MODE, PULL_UP, inputCallback);
	
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
			// Setup alarm
			alarm(1);
			break;
		case STATUS_READY:
			// Start race
			printf("Go !\n");
			time0 = timeInterrupt;
			setStatus(&status, STATUS_RACE);
			break;
	}
}

// Listening to messages thread
void * messagesThread() {
	while (1) {
		waitForMessage(sd, &timeMess, sizeof(timeMess));
		if (getStatus(&status) == STATUS_RACE) {
			printf("End race !\n");
			
			printf("Time  : %.2lf s\n", computeTime(time0, timeMess));
			setStatus(&status, STATUS_WAITING);
		}
	}
	pthread_exit(NULL);
}

// Manage signals
void signalsHandler(int sigNum) {
	switch(sigNum) {
		case SIGALRM:
			// Check if the pedal is pressed and if we are waiting to start a race
			#if BUTTON_MODE_INV
			if (getStatus(&status) == STATUS_WAITING && !getGPIOValue(PIN)) {
			#else
			if (getStatus(&status) == STATUS_WAITING && getGPIOValue(PIN)) {
			#endif
				setStatus(&status, STATUS_READY);
				printf("Ready...\n");
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


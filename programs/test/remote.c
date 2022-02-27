#include "../../include/gpioLib.h"
#include "../../include/socketLib.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define PIN 16
#define EXTERN_PULL_UP 0

#define SERVER_PORT 1234
#define CLIENT_PORT 1235
#define SERVER_IP "172.24.1.1"

void callback(int gpio, int level, uint32_t tick);

struct timespec time1;

int sd;

int main(void) {
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	
	clock_gettime(CLOCK_REALTIME, &time1);
	
	printf("Init GPIO\n");
	initGPIO();
	#if EXTERN_PULL_UP
		setupGPIOInput(PIN, RISING_MODE, NO_PULL_UP, callback);
	#else
		setupGPIOInput(PIN, RISING_MODE, PULL_UP, callback);
	#endif
	
	printf("Waiting for connection...\n");
	clientConnect(&sd, &clientAddr, CLIENT_PORT, &serverAddr, SERVER_IP, SERVER_PORT);
	printf("Connected !\n");
	
	while (pause());
	return 0;
}

void callback(int gpio, int level, uint32_t tick) {
	clock_gettime(CLOCK_REALTIME, &time1);
	printf("Button detected !\n");
	sendMessage(sd, &time1, sizeof(time1));	
}


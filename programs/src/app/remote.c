#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../../config/remote.h"
#include "../../include/gpioLib.h"
#include "../../include/socketLib.h"
#include "../../include/check.h"
#include "../../include/messages.h"

#define SERVER_PORT 1234
#define CLIENT_PORT 1235
#define SERVER_PORT_PING 4321
#define CLIENT_PORT_PING 4322
#define SERVER_IP "172.24.1.1"

#define INPUT_DELAY 20000 // In micro seconds

#define PING_DELAY 3 // In seconds
#define PING_TIMEOUT 8 // In seconds

// Time when an interrupt occurs
struct timespec timeInterrupt;
struct timespec timeInterrupt1;
struct timespec timeInterrupt2;
int btn1State = 0;
int btn2State = 0;
int listenBtn1 = 1;
int listenBtn2 = 1;


int sd;
raceMessage message;
pingMessage pingInfo;

void inputCallback(int gpio, int level, uint32_t tick);
void * pingThread();

int main(void) {
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	pthread_t threadPing;

    // Init GPIO
	initGPIO();
    setupGPIOInput(PIN_BTN_1, BOTH_RISING_FALLING_MODE, BTN_1_PULL_UP_RESISTOR, inputCallback);
    setupGPIOInput(PIN_BTN_2, BOTH_RISING_FALLING_MODE, BTN_1_PULL_UP_RESISTOR, inputCallback);
    btn1State = getGPIOValue(PIN_BTN_1, BTN_1_REVERSE_INPUT);
    btn2State = getGPIOValue(PIN_BTN_2, BTN_2_REVERSE_INPUT);

    while (1) {
        // Connexion to spidal
        printf("Connexion ...\n");
        clientConnect(&sd, &clientAddr, CLIENT_PORT, &serverAddr, SERVER_IP, SERVER_PORT);
        printf("Connected !\n");

        // Creating ping thread
        CHECK_T(
            pthread_create(&threadPing, NULL, pingThread, NULL),
            "--pthread_create()"
        );

        // Wait for ping thread end meaning that we lost connexion
        CHECK_T(pthread_join(threadPing, NULL), "--pthread_join()");
        printf("Lost connexion ...\n");
        close(sd);
    }

	return EXIT_SUCCESS;
}

// Manage inputs interrupts
void inputCallback(int gpio, int level, uint32_t tick) {
	clock_gettime(CLOCK_REALTIME, &timeInterrupt);
	if (listenBtn1 && gpio == PIN_BTN_1) {
        listenBtn1 = 0;
        timeInterrupt1 = timeInterrupt;
        usleep(INPUT_DELAY);
        if (btn1State != getGPIOValue(gpio, BTN_1_REVERSE_INPUT)) {
            btn1State = getGPIOValue(gpio, BTN_1_REVERSE_INPUT);
            message.pId = 1;
            message.time = timeInterrupt1;
            message.state = btn1State;
            printf("Sending BTN1 %s\n", (btn1State? "ON" : "OFF"));
	        sendMessage(sd, &message, sizeof(message));	
        }
        listenBtn1 = 1;
    } else if (listenBtn2 && gpio == PIN_BTN_2) {
        listenBtn2 = 0;
        timeInterrupt2 = timeInterrupt;
        usleep(INPUT_DELAY);
        if (btn2State != getGPIOValue(gpio, BTN_2_REVERSE_INPUT)) {
            btn2State = getGPIOValue(gpio, BTN_2_REVERSE_INPUT);
            message.pId = 2;
            message.time = timeInterrupt2;
            message.state = btn2State;
            printf("Sending BTN2 %s\n", (btn2State? "ON" : "OFF"));
	        sendMessage(sd, &message, sizeof(message));	
        }
        listenBtn2 = 1;
    }
}

// Ping Thread
void * pingThread() {
	int sdPing;
    fd_set readFd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	pingInfo.i = 1;

    struct timeval timeout = { PING_TIMEOUT, 0 };
    int err;
	
	clientConnect(&sdPing, &clientAddr, CLIENT_PORT_PING, &serverAddr, SERVER_IP, SERVER_PORT_PING);
        
    FD_ZERO(&readFd);
    FD_SET(sdPing, &readFd);

    sendMessage(sdPing, &pingInfo, sizeof(pingInfo));

    while (1) {
        timeout.tv_sec = PING_TIMEOUT;
        timeout.tv_usec = 0;
        err = select(sdPing + 1, &readFd, NULL, NULL, &timeout);
        //printf("select ok\n");
        if (err <= 0) {
            // Error or timeout
	        close(sdPing);
	        pthread_exit(NULL);
        } else if (err > 0) {
            if (!waitForMessage(sdPing, &pingInfo, sizeof(pingInfo))) {
                // Lost connexion
                close(sdPing);
                pthread_exit(NULL);
            }
            //printf("recv ping\n");
            sleep(PING_DELAY);
            sendMessage(sdPing, &pingInfo, sizeof(pingInfo));
            //printf("Send ping\n");
            sleep(1);
        }
    }
}
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "../../config/spidal.h"
#include "../../include/gpioLib.h"
#include "../../include/socketLib.h"
#include "../../include/check.h"
#include "../../include/messages.h"
#include "../../include/signalsLib.h"

#define SERVER_PORT 1234
#define SERVER_PORT_PING 4321

#define INPUT_DELAY 20000 // In micro seconds

#define PING_DELAY 3
#define PING_TIMEOUT 8 // In seconds

#define MAX_DATA_LEN 100

// Time when an interrupt occurs
struct timespec timeInterrupt;
struct timespec timeInterrupt1;
struct timespec timeInterrupt2;
int pedal1State = 0;
int pedal2State = 0;
int listenPedal1 = 1;
int listenPedal2 = 1;

// Reading socket

int se;
int sd;
int sePing;
int sdPing;
raceMessage message;
pingMessage pingInfo;

char data[MAX_DATA_LEN];
int dataLength;

pthread_t threadMessages;
pthread_t threadPing;

int out;

void inputCallback(int gpio, int level, uint32_t tick);
void * messagesThread();
void * pingThread();
void sendData();
//void signalsHandler(int sigNum);

int main(int argc, char * argv[]) {
	struct sockaddr_in serverAddr;

    if (argc != 2) {
        out = STDOUT_FILENO;
    } else {
        out = atoi(argv[1]);
    }

    // Install signals handler
	// installSignalHandler(SIGTERM, signalsHandler);
	// installSignalHandler(SIGINT, signalsHandler);

    // Init GPIO
	initGPIO();
	setupGPIOInput(PIN_PEDAL_1, BOTH_RISING_FALLING_MODE, PEDAL_1_PULL_UP_RESISTOR, inputCallback);
	setupGPIOInput(PIN_PEDAL_2, BOTH_RISING_FALLING_MODE, PEDAL_2_PULL_UP_RESISTOR, inputCallback);
    pedal1State = getGPIOValue(PIN_PEDAL_1, PEDAL_1_REVERSE_INPUT);
    pedal2State = getGPIOValue(PIN_PEDAL_2, PEDAL_2_REVERSE_INPUT);

    while (1) {
        // Connexion to spidalremote
        serverConnect(&se, &sd, &serverAddr, SERVER_PORT);
        dataLength = sprintf(data, "REMOTE:1:0:0\n");
        sendData();

        // Creating waiting message thread
        CHECK_T(
            pthread_create(&threadMessages, NULL, messagesThread, NULL),
            "--pthread_create()"
        );

        // Creating ping thread
        CHECK_T(
            pthread_create(&threadPing, NULL, pingThread, NULL),
            "--pthread_create()"
        );

        // Wait for ping thread end meaning that we lost connexion
        CHECK_T(pthread_join(threadPing, NULL), "--pthread_join()");
        dataLength = sprintf(data, "REMOTE:0:0:0\n");
        sendData();
        pthread_cancel(threadMessages);
        close(sd);
	    close(se);
    }
    
	CHECK_T(pthread_join(threadMessages, NULL), "--pthread_join()");
	close(sd);
	close(se);
    return EXIT_SUCCESS;
}

// Manage inputs interrupts
void inputCallback(int gpio, int level, uint32_t tick) {
	clock_gettime(CLOCK_REALTIME, &timeInterrupt);
	if (listenPedal1 && gpio == PIN_PEDAL_1) {
        listenPedal1 = 0;
        timeInterrupt1 = timeInterrupt;
        usleep(INPUT_DELAY);
        if (pedal1State != getGPIOValue(gpio, PEDAL_1_REVERSE_INPUT)) {
            pedal1State = getGPIOValue(gpio, PEDAL_1_REVERSE_INPUT);
            dataLength = sprintf(data, "PEDAL1:%s:%ld:%ld\n", (pedal1State ? "1" : "0"), timeInterrupt1.tv_sec, timeInterrupt1.tv_nsec);
            sendData();
        }
        listenPedal1 = 1;
    } else if (listenPedal2 && gpio == PIN_PEDAL_2) {
        listenPedal2 = 0;
        timeInterrupt2 = timeInterrupt;
        usleep(INPUT_DELAY);
        if (pedal2State != getGPIOValue(gpio, PEDAL_2_REVERSE_INPUT)) {
            pedal2State = getGPIOValue(gpio, PEDAL_2_REVERSE_INPUT);
            dataLength = sprintf(data, "PEDAL2:%s:%ld:%ld\n", (pedal2State ? "1" : "0"), timeInterrupt2.tv_sec, timeInterrupt2.tv_nsec);
            sendData();
        }
        listenPedal2 = 1;
    }
}

// Thread listening to messages
void * messagesThread() {
	while (1) {
		if (waitForMessage(sd, &message, sizeof(message))) {
		    dataLength = sprintf(data, "BTN%d:%s:%ld:%ld\n", message.pId, (message.state ? "1" : "0"), message.time.tv_sec, message.time.tv_nsec);
            sendData();
        }
    }
	pthread_exit(NULL);
}

// Ping Thread
void * pingThread() {
    fd_set readFd;
	struct sockaddr_in serverAddr;

	pingInfo.i = 1;

    struct timeval timeout = { PING_TIMEOUT, 0 };
    int err;
	
	serverConnect(&sePing, &sdPing, &serverAddr, SERVER_PORT_PING);

    FD_ZERO(&readFd);
    FD_SET(sdPing, &readFd);
	
    while (1) {
        timeout.tv_sec = PING_TIMEOUT;
        timeout.tv_usec = 0;
        err = select(sdPing + 1, &readFd, NULL, NULL, &timeout);
        //printf("%ld : %ld\n", timeout.tv_sec, timeout.tv_usec);
        if (err <= 0) {
            // Error or timeout
            close(sePing);
	        close(sdPing);
	        pthread_exit(NULL);
        } else if (err > 0) {
            if (!waitForMessage(sdPing, &pingInfo, sizeof(pingInfo))) {
                // Lost connexion
                close(sePing);
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

void sendData() {
    write(out, data, dataLength+1);
}

// Manage signals
// void signalsHandler(int sigNum) {
// 	switch(sigNum) {
// 		case SIGTERM:
//         case SIGINT:
// 			// Close everything
//             pthread_cancel(threadMessages);
//             pthread_cancel(threadPing);
//             close(sd);
//             close(se);
//             close(sdPing);
//             close(sePing);

//             exit(EXIT_SUCCESS);
// 			break;
// 	}
// }
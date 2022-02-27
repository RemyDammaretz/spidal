#ifndef APP_STATUS
#define APP_STATUS

#include <pthread.h>

// Previous status
#define STATUS_START 0
#define STATUS_WAITING 1
#define STATUS_READY 2
#define STATUS_RACE 3

// App pages
#define APP_MENU 10
#define APP_FREE_RACE 11
#define APP_TRAINING_SOLO 12
#define APP_TRAINING_DUO 13

// Runner status
#define RUNNER_WAITING 20
#define RUNNER_READY 21
#define RUNNER_RACE 22
#define RUNNER_END_RACE 23

// Global Race status
#define RACE_WAITING 30
#define RACE_READY 31
#define RACE_DECOUNT 32
#define RACE_ONGOING 33
#define RACE_END 34

typedef struct {
	int status;
	pthread_mutex_t mutex;
} status_t; 

void initStatus(status_t * status, int value);

void setStatus(status_t * status, int value);

int getStatus(status_t * status);

#endif

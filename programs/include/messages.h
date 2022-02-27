#ifndef MESSAGES
#define MESSAGES

#include <time.h>

typedef struct {
	int pId;
	struct timespec time;
	int state;
} raceMessage;

typedef struct {
	int i;
} pingMessage;

#endif

#include "../../include/signalsLib.h"
#include "../../include/check.h"
#include <stdlib.h>


void installSignalHandler(int sigNum, void (* handler)(int)) {
	struct sigaction action;
	action.sa_handler = handler;
	CHECK(sigemptyset(&action.sa_mask),"--sigemptyset()");
	action.sa_flags = 0;
	CHECK(sigaction(sigNum, &action, NULL), "--sigaction()");
}

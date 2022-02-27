#ifndef SIGNALS_LIB

#define SIGNALS_LIB

#include <signal.h>

void installSignalHandler(int sigNum, void (* handler)(int));

#endif

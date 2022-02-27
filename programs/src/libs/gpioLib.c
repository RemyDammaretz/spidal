#include "../../include/gpioLib.h"
#include <pigpio.h>

void initGPIO() {
	gpioInitialise();
}

void setupGPIOInput(int pin, int mode, int pullUp, interruptCallback callback) {
	gpioSetMode(pin, PI_INPUT);
	if (pullUp == PULL_UP) 
		gpioSetPullUpDown(pin, PI_PUD_UP);
	switch (mode) {
		case FALLING_MODE:
			gpioSetISRFunc(pin, FALLING_EDGE, 0, callback);
		break;
		case RISING_MODE:
			gpioSetISRFunc(pin, RISING_EDGE, 0, callback);
		break;
		case BOTH_RISING_FALLING_MODE:
			gpioSetISRFunc(pin, EITHER_EDGE, 0, callback);
		break;
	}
}

int getGPIOValue(int pin, int reverseInput) {
	if (reverseInput == NO_REVERSE_INPUT) return !gpioRead(pin);
	else return gpioRead(pin);
}

void stopGPIO() {
	gpioTerminate();
}



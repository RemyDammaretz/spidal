#ifndef GPIO_LIB

#define GPIO_LIB

#include <pigpio.h>

#define RISING_MODE 0
#define FALLING_MODE 1
#define BOTH_RISING_FALLING_MODE 2

// Use PULL_UP with raspberry pi B 
// Use NO_PULL_UP with raspberry zero
#define PULL_UP 0
#define NO_PULL_UP 1

// Reverse inputs
// Should be used when pressing a button/pedal physically opens the electrical circuit instead of closing it
#define REVERSE_INPUT 0
#define NO_REVERSE_INPUT 1

typedef void (*interruptCallback) (int gpio, int level, uint32_t tick);

void initGPIO();
void setupGPIOInput(int pin, int mode, int pullUp, interruptCallback callback);
int getGPIOValue(int pin, int reverseInput);
void stopGPIO();

#endif

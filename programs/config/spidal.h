#ifndef SPIDAL_H
#define SPIDAL_H

// Screen resolution
#define SCREEN_RESOLUTION_X 800
#define SCREEN_RESOLUTION_Y 480

// Default path
#ifndef NO_RPI
    #define PATH "/home/pi/spidal/programs/"
#else
    // Default path for tests on computer without Raspberry Pi
    #define PATH ""
#endif

#ifndef NO_RPI

#include "../include/gpioLib.h"

// GPIO
#define PIN_PEDAL_1 16
#define PIN_PEDAL_2 13

// Use internal pull up resistor on pedals ?
// PULL_UP : Yes
// NO_PULL_UP : No
#define PEDAL_1_PULL_UP_RESISTOR PULL_UP
#define PEDAL_2_PULL_UP_RESISTOR PULL_UP

// Inverse pedals inputs
// Should be used when pressing a pedal physically opens the electrical circuit instead of closing it (like some emergency stop buttons)
// Values :
// REVERSE_INPUT
// NO_REVERSE_INPUT
#define PEDAL_1_REVERSE_INPUT NO_REVERSE_INPUT
#define PEDAL_2_REVERSE_INPUT NO_REVERSE_INPUT

#endif

#endif
#ifndef REMOTE_H
#define REMOTE_H

#include "../include/gpioLib.h"

// GPIO
#define PIN_BTN_1 16
#define PIN_BTN_2 13

// Use internal pull up resistor on buttons ?
// Values :
// PULL_UP
// NO_PULL_UP
#define BTN_1_PULL_UP_RESISTOR PULL_UP
#define BTN_2_PULL_UP_RESISTOR PULL_UP

// Inverse buttons inputs
// Should be used when pressing a button physically opens the electrical circuit instead of closing it (like some emergency stop buttons)
// Values :
// REVERSE_INPUT
// NO_REVERSE_INPUT
#define BTN_1_REVERSE_INPUT REVERSE_INPUT
#define BTN_2_REVERSE_INPUT REVERSE_INPUT

#endif
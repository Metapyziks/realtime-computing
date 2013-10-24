/**
 * File Name  : input.h
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : Provides a method to get the last key pressed since the
 *              previous call to that method (if any), and another that waits
 *              for a key to be pressed before returning it.
 */

#ifndef INPUT_H_GUARD
#define INPUT_H_GUARD

//////////////
// Includes //
//////////////

#include "utils.h"

///////////////////////
// Const Definitions //
///////////////////////

// The number of possible buttons that can be pressed.
#define BUTTON_COUNT 5

// Bit numbers in FIO0PIN for each button.
#define BUTTON_NONE -1
#define BUTTON_UP 10
#define BUTTON_DOWN 11
#define BUTTON_LEFT 12
#define BUTTON_RIGHT 13
#define BUTTON_CENTER 22

//////////////////////////
// Function Definitions //
//////////////////////////

// Checks to see if a button has been pressed since the last time this function
// was called, and returns that button's ID if one has. If no button has been
// pressed, returns BUTTON_NONE.
int input_getButtonPress()
{
    int i, curr, diff;

    // Record the previous state of ~FIO0PIN between invocations.
    static int prev = 0;

    // List of button IDs to loop through.
    const int buttons[BUTTON_COUNT] = {
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_CENTER
    };

    // Why on earth does FIO0PIN use a 0 to signify a button being pressed?
    curr = ~FIO0PIN;

    // Find the buttons that have been pressed since last invocation.
    diff = (curr ^ prev) & curr;

    // Remember the current button state for next time.
    prev = curr;

    // Find the first button that has just been pressed and return it.
    for (i = 0; i < BUTTON_COUNT; ++i) {
        if (getBit(diff, buttons[i])) return buttons[i];
    }

    // Otherwise, nothing new has been pressed.
    return BUTTON_NONE;
}

// Blocks until a button is pressed, and then returns that buttons's ID.
int input_waitForButtonPress()
{
    int btn;
    
    // Super condensed.
    while ((btn = input_getButtonPress()) == BUTTON_NONE);
    return btn;
}

#endif

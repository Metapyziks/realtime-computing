#ifndef INPUT_H_GUARD
#define INPUT_H_GUARD

#include "utils.h"

#define BUTTON_NONE 0
#define BUTTON_UP 10
#define BUTTON_DOWN 11
#define BUTTON_LEFT 12
#define BUTTON_RIGHT 13
#define BUTTON_CENTER 22

int _oldButtons = 0;
int input_getButtonPress()
{
    int curr, diff;

    curr = ~FIO0PIN;
    diff = (curr ^ _oldButtons) & curr;
    _oldButtons = curr;

    if (getBit(diff, BUTTON_UP)) return BUTTON_UP;
    if (getBit(diff, BUTTON_DOWN)) return BUTTON_DOWN;
    if (getBit(diff, BUTTON_LEFT)) return BUTTON_LEFT;
    if (getBit(diff, BUTTON_RIGHT)) return BUTTON_RIGHT;
    if (getBit(diff, BUTTON_CENTER)) return BUTTON_CENTER;

    return 0;
}

int input_waitForButtonPress()
{
    int btn;

    for (;;) {
        btn = input_getButtonPress();
        if (btn != BUTTON_NONE) return btn;
    }
}

#endif

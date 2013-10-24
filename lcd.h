/**
 * File Name  : lcd.h
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : A couple of handy functions related to drawing text on the LCD
 *              display.
 */

#ifndef LCD_H_GUARD
#define LCD_H_GUARD

//////////////
// Includes //
//////////////

#include <lcd_grph.h>

#include "utils.h"

///////////////////////
// Const Definitions //
///////////////////////

// Width of each character when drawn to the display, in pixels.
#define CHAR_WIDTH 6

// Height of each character when drawn to the display, in pixels.
#define CHAR_HEIGHT 8

//////////////////////
// Type Definitions //
//////////////////////

// Structure representing a position in 2D space.
typedef struct {
    int x, y;
} Point;

// Structure representing the size of an object in 2D space.
typedef struct {
    int width, height;
} Size;

// Structure representing the combined position and size of a rectangle.
typedef struct {
    Point pos;
    Size size;
} Rect;

//////////////////////////
// Function Definitions //
//////////////////////////

// Calculates the width and height of the given string if it were to be drawn
// on the LCD display.
Size lcd_getStringSize(char* str)
{
    Size size;

    // I should probably take newlines into account some day...
    size.width = strlen(str) * CHAR_WIDTH;
    size.height = CHAR_HEIGHT;

    return size;
}

// Draws the given string in the centre of the provided rectangle, as defined
// by its top-left position, width, and height. Returns a Rect structure
// containing the position and size of the drawn text.
Rect lcd_putStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str)
{
    Rect rect;

    // Find the size of the text.
    rect.size = lcd_getStringSize(str);

    // Position the text taking the size into account.
    rect.pos.x = x + ((w - rect.size.width) >> 1);
    rect.pos.y = y + ((h - rect.size.height) >> 1);

    // Draw the text at the calculated position.
    lcd_putString(rect.pos.x, rect.pos.y, str);

    return rect;
}

#endif

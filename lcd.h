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
#include <font5x7.h>

#include "utils.h"

///////////////////////
// Const Definitions //
///////////////////////

// Width of each character when drawn to the display, in pixels.
#define CHAR_WIDTH 6
#define BIG_CHAR_WIDTH 12

// Height of each character when drawn to the display, in pixels.
#define CHAR_HEIGHT 8
#define BIG_CHAR_HEIGHT 16

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

///////////////////////////
// Function Declarations //
///////////////////////////

bool lcd_putBigChar(unsigned short x, unsigned short y, char chr);
void lcd_putBigString(unsigned short x, unsigned short y, char *pStr);

Size lcd_getStringSize(char* str);
Rect lcd_putStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str);

Size lcd_getBigStringSize(char* str);
Rect lcd_putBigStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str);

//////////////////////////
// Function Definitions //
//////////////////////////

bool lcd_putBigChar(unsigned short x, unsigned short y, char chr)
{  
    static unsigned char const font_mask[8] =
        {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    unsigned char data = 0;
    unsigned char i = 0, j = 0;
    unsigned char ch = (unsigned char) chr & 0x7f;

    lcd_color_t color = BLACK;

    if((x >= (DISPLAY_WIDTH - BIG_CHAR_WIDTH)) || (y >= (DISPLAY_HEIGHT - BIG_CHAR_HEIGHT))) return FALSE;

    if((ch < 0x20) || (ch > 0x7f)) ch = 0x20;

    ch -= 0x20;
    for (i = 0; i < BIG_CHAR_HEIGHT; ++i) {
        data = font5x7[ch][i >> 1];

        for (j = 0; j < BIG_CHAR_WIDTH; ++j) {
            if((data & font_mask[j >> 1]) == 0) {  
                color = BLACK;
            } else {
                color = WHITE;
            }
            lcd_point(x, y, color);       
            ++x;
        }   
        ++y;
        x -= BIG_CHAR_WIDTH;
    }

    return TRUE;
}

void lcd_putBigString(unsigned short x, unsigned short y, char *pStr)
{
    for (;;) {      
        if((*pStr) == '\0') return;
        if(!lcd_putBigChar(x, y, *pStr++)) return;
        x += BIG_CHAR_WIDTH;
    }
}

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

Size lcd_getBigStringSize(char* str)
{
    Size size;

    // I should probably take newlines into account some day...
    size.width = strlen(str) * BIG_CHAR_WIDTH;
    size.height = BIG_CHAR_HEIGHT;

    return size;
}

// Draws the given string in the centre of the provided rectangle, as defined
// by its top-left position, width, and height. Returns a Rect structure
// containing the position and size of the drawn text.
Rect lcd_putBigStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str)
{
    Rect rect;

    // Find the size of the text.
    rect.size = lcd_getBigStringSize(str);

    // Position the text taking the size into account.
    rect.pos.x = x + ((w - rect.size.width) >> 1);
    rect.pos.y = y + ((h - rect.size.height) >> 1);

    // Draw the text at the calculated position.
    lcd_putBigString(rect.pos.x, rect.pos.y, str);

    return rect;
}

#endif

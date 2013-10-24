#ifndef LCD_H_GUARD
#define LCD_H_GUARD

#include <lcd_grph.h>

#include "utils.h"

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8

typedef struct {
    int x, y;
} Point;

typedef struct {
    int width, height;
} Size;

typedef struct {
    Point pos;
    Size size;
} Rect;

Size lcd_getStringSize(char* str)
{
    Size size;
    size.width = strlen(str) * CHAR_WIDTH;
    size.height = CHAR_HEIGHT;
    return size;
}

Rect lcd_putStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str)
{
    Rect rect;

    rect.size = lcd_getStringSize(str);
    rect.pos.x = x + (w - rect.size.width) / 2;
    rect.pos.y = y + (h - rect.size.height) / 2;

    lcd_putString(rect.pos.x, rect.pos.y, str);

    return rect;
}

#endif

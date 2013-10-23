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

char* toString(int var)
{
    int chars, max, i;
    char* buffer;

    max = 10;
    for (chars = 1; max <= var; ++chars) max *= 10;

    buffer = (char*) malloc((chars + 1) * sizeof(char));

    buffer[chars] = '\0';

    for (i = chars - 1; i >= 0; --i) {
        buffer[i] = (char) (48 + (var % 10));
        var /= 10;
    }

    return buffer;
}

#endif

#include <string.h>

#ifdef EXTMEM
#include <lcd_grph.h>
#endif

#define BUTTON_NONE 0
#define BUTTON_UP 10
#define BUTTON_DOWN 11
#define BUTTON_LEFT 12
#define BUTTON_RIGHT 13
#define BUTTON_CENTER 22

#define DELAY_MULT 3000

#define bitMask(c) ((1 << (c)) - 1)
 
#define getBit(x, i) (((x) >> (i)) & 1)
#define getBits(x, i, c) (((x) >> (i)) & bitMask((c)))
 
#define clrBit(x, i) ((x) & ~(1 << (i)))
#define setBit(x, i) ((x) | (1 << (i)))
#define cpyBit(x, i, v) (clrBit(x, i) | (((v) & 1) << i))
 
#define clrBits(x, i, c) ((x) & ~(bitMask(c) << (i)))
#define setBits(x, i, c) ((x) | (bitMask(c) << (i)))
#define cpyBits(x, i, c, v) (clrBits(x, i, c) | (((v) & bitMask(c)) << (i)))

#ifdef EXTMEM

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

#else

int _oldButtons = 0;
int getButtonPress()
{
	int curr, diff;

	curr = FIO0PIN;
	diff = (curr ^ _oldButtons) & curr;
	_oldButtons = curr;

	if (getBit(diff, BUTTON_UP)) return BUTTON_UP;
	if (getBit(diff, BUTTON_DOWN)) return BUTTON_DOWN;
	if (getBit(diff, BUTTON_LEFT)) return BUTTON_LEFT;
	if (getBit(diff, BUTTON_RIGHT)) return BUTTON_RIGHT;
	if (getBit(diff, BUTTON_CENTER)) return BUTTON_CENTER;

	return 0;
}

int waitForButtonPress()
{
	int btn;

	for (;;) {
		btn = getButtonPress();
		if (btn != BUTTON_NONE) return btn;
	}
}

#endif

void wait(int millis)
{
	volatile int i = 0;
	for (i = 0; i < millis * DELAY_MULT; ++i);
}

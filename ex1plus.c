#include <lcd_grph.h>
#include "utils.h"

int main()
{
	Rect rect;

	lcd_init();
	
	rect = lcd_putStringCentered(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, "Hello world!");

	lcd_drawRect(rect.pos.x - 2, rect.pos.y - 2,
		rect.size.width + rect.pos.x - 4,
		rect.size.height + rect.pos.y,
		WHITE);

	return 0;
}

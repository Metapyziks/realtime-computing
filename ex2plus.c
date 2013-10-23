#include "input.h"

int main()
{
	int pos = 0;
	int config = 0;
	int flash = 0;

	FIO3DIR = 0xFFFFFFFF;

	for (;;) {
		switch (input_getButtonPress()) {
			case BUTTON_LEFT:
				pos = (pos + 1) & 31;
				break;
			case BUTTON_RIGHT:
				pos = (pos - 1) & 31;
				break;
			case BUTTON_UP:
				config = setBit(config, pos);
				break;
			case BUTTON_DOWN:
				config = clrBit(config, pos);
				break;
			case BUTTON_CENTER:
				config = cpyBit(config, pos, 1 ^ getBit(config, pos));
				break;
			case BUTTON_NONE:
				FIO3PIN = cpyBit(config, pos, flash < 125);
				flash = (flash + 1) % (250);
				wait(1);
				break;
		}
	}

	return 0;
}

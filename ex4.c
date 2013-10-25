
#include "utils.h"
#include "input.h"

#define WAVE_TRIANGLE 0
#define WAVE_SQUARE 1
#define WAVE_SINE 2

int main(void)
{
	int i; int w; int d; int v; bool playing;

	PINSEL1 = cpyBits(PINSEL1, 20, 2, 1 << 1);

	playing = FALSE;
	w = WAVE_SINE;
	d = 1;

	for (i = 0;;i = (i + 1) % 65536) {
		switch (input_getButtonPress()) {
			case BUTTON_CENTER:
				playing = !playing;
				break;
			case BUTTON_UP:
				d = min(16, d + 1);
				break;
			case BUTTON_DOWN:
				d = max(1, d - 1);
				break;
		}

		if (playing) {
			v = ((i * d) / 16) & 0x1ff;

			switch (w) {
				case WAVE_TRIANGLE:
					if (v & 0x100) {
						DACR = cpyBits(0, 6, 10, v & 0xff);
					} else {
						DACR = cpyBits(0, 6, 10, ~v & 0xff);
					}
					break;
				case WAVE_SQUARE:
					DACR = cpyBits(0, 6, 10, v > 0xff ? 0xff : 0x00);
					break;
				case WAVE_SINE:
					DACR = cpyBits(0, 6, 10, (int) (sin(v * PI / 32.0) * 127) + 128);
					break;
			}
		}
	}

	return 0;
}

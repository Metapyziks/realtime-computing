#include "utils.h"
#include "lcd.h"
#include "input.h"

#define SAMPLE_PERIOD 4
#define SAMPLE_RATE 16000
#define SAMPLE_LENGTH (SAMPLE_RATE * SAMPLE_PERIOD)

void adc_init()
{
	PINSEL1 = cpyBits(PINSEL1, 16, 2, 1);
	PCONP = setBit(PCONP, 12);

	AD0CR = setBit(cpyBits(2, 8, 8, 12000000 / (SAMPLE_RATE * 16)), 21);
}

void adc_kill()
{
	AD0CR = clrBit(AD0CR, 21);
}

short adc_read()
{
	AD0CR = cpyBits(AD0CR, 24, 3, 1);
	while (!getBit(AD0DR1, 31));
	return (short) getBits(AD0DR1, 6, 10);
}

void dac_init()
{
	PINSEL1 = cpyBits(PINSEL1, 20, 2, 1 << 1);
}

void clear(char* buffer)
{
	int i;

	for (i = 0; i < SAMPLE_LENGTH; ++i) {
		buffer[i] = 0;
	}
}

void record(char* buffer)
{
	int y, b; short val;

	lcd_fillScreen(RED);
	for (y = 0, b = 0; b < SAMPLE_LENGTH; y = (y + 1) & 0xff, ++b) {
		val = adc_read();
		buffer[b] = (char) (val >> 2);
	}
	lcd_fillScreen(BLACK);
}

void play(char* buffer, int volume)
{
	int y, b; int val; volatile int i;

	lcd_fillScreen(WHITE);
	for (y = 0, b = 0; b < SAMPLE_LENGTH; y = (y + 1) & 0xff, ++b) {
		val = ((buffer[b] << 1) * volume) / 50;
		DACR = cpyBits(0, 6, 10, val);
		for (i = 0; i < 180; ++ i);
	}
	lcd_fillScreen(BLACK);
}

int main(void)
{
	char buffer[SAMPLE_LENGTH]; int volume;

	lcd_init();
	dac_init();
	adc_init();

	clear(buffer);

	volume = 50;

	for(;;) {
		switch (input_waitForButtonPress()) {
			case BUTTON_CENTER:
				record(buffer);
				break;
			case BUTTON_LEFT:
				clear(buffer);
				break;
			case BUTTON_RIGHT:
				play(buffer, volume);
				break;
			case BUTTON_UP:
				volume = min(100, volume + 5);
				break;
			case BUTTON_DOWN:
				volume = max(0, volume - 5);
				break;
		}
	}

	return 0;
}

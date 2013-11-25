#include "utils.h"
#include "lcd.h"
#include "input.h"

#define SAMPLE_PERIOD 5
#define SAMPLE_RATE 12000
#define PLAYBACK_LOOP_ITERS (3000000 / SAMPLE_RATE)
#define SAMPLE_LENGTH (SAMPLE_RATE * SAMPLE_PERIOD)

void adc_init(void);
void adc_kill(void);
short adc_read(void);

void dac_init(void);

void clear(char* buffer);
void record(char* buffer);
void play(char* buffer, int volume);

void drawBuffer(char* buffer, int x, int y, int w, int h);
void drawVolume(int volume, int x, int y, int w, int h);

void adc_init(void)
{
    PINSEL1 = cpyBits(PINSEL1, 16, 2, 1);
    PCONP = setBit(PCONP, 12);

    AD0CR = setBit(cpyBits(2, 8, 8, 12000000 / (SAMPLE_RATE * 16)), 21);
}

void adc_kill(void)
{
    AD0CR = clrBit(AD0CR, 21);
}

short adc_read(void)
{
    AD0CR = cpyBits(AD0CR, 24, 3, 1);
    while (!getBit(AD0DR1, 31));
    return (short) getBits(AD0DR1, 6, 10);
}

void dac_init(void)
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
    int b, x, oldX; short val;

    lcd_fillRect(4, 126, DISPLAY_WIDTH - 6, 128, WHITE);
    oldX = 0;
    for (b = 0; b < SAMPLE_LENGTH; ++b) {
        val = adc_read();
        buffer[b] = (char) (val >> 2);

        x = (b * (DISPLAY_WIDTH - 19)) / SAMPLE_LENGTH;
        if (x > oldX) {
            oldX = x;
            lcd_fillRect(4 + x - 1, 126, 4 + x, 128, RED);
        }

    }
    lcd_fillRect(4, 4, DISPLAY_WIDTH - 6, 126, BLACK);
}

void play(char* buffer, int volume)
{
    int b; int val; volatile int i;

    for (b = 0; b < SAMPLE_LENGTH; ++b) {
        val = (((int) buffer[b] << 1) * volume) / 256;
        DACR = cpyBits(0, 6, 10, val);
        for (i = 0; i < PLAYBACK_LOOP_ITERS; ++ i);
    }
}

void drawBuffer(char* buffer, int x, int y, int w, int h)
{
    short val, prev, rangeMin, rangeMax, valMin, valMax;
    int i, b, start, end;
    long int avg;

    rangeMin = 0x3ff;
    rangeMax = 0x000;
    avg = 0;

    for (b = 0; b < SAMPLE_LENGTH; ++b) {
        val = buffer[b];
        avg += val;
        
        if (val > rangeMax) rangeMax = val;
        if (val < rangeMin) rangeMin = val;
    }

    avg /= SAMPLE_LENGTH;

    rangeMax = max(rangeMax - avg, avg - rangeMin);

    prev = 0;
    for (i = 0; i < w; ++i) {
        start = (i * SAMPLE_LENGTH) / w;
        end = ((i + 1) * SAMPLE_LENGTH) / w;

        if (start == end) continue;

        valMin = 0x3ff;
        valMax = 0x000;
        for (b = start; b < end; ++b) {
            val = buffer[b];
            if (val < valMin) valMin = val;
            if (val > valMax) valMax = val;
        }

        val = max(abs(valMax - avg), abs(avg - valMin));

        lcd_line(x + i - 1, y + h - (prev * h) / rangeMax,
            x + i, y + h - (val * h) / rangeMax, WHITE);

        prev = val;
    }
}

void drawVolume(int volume, int x, int y, int w, int h)
{
    lcd_fillRect(x, y, x + w, y + h - (volume * h) / 9, WHITE);
    lcd_fillRect(x, y + h - (volume * h) / 9, x + w, y + h, RED);
}

int main(void)
{
    char buffer[SAMPLE_LENGTH]; int volume;

    const int volumes[10] = {
        0, 16, 22, 31, 44, 61, 86, 120, 169, 256
    };

    lcd_init();
    dac_init();
    adc_init();

    lcd_putBigStringCentered(0, 160, DISPLAY_WIDTH, 32,
        "Center : Record  ");
    lcd_putBigStringCentered(0, 192, DISPLAY_WIDTH, 32,
        " Right : Play    ");
    lcd_putBigStringCentered(0, 224, DISPLAY_WIDTH, 32,
        "    Up : +Volume ");
    lcd_putBigStringCentered(0, 256, DISPLAY_WIDTH, 32,
        "  Down : -Volume ");

    clear(buffer);

    volume = 8;
    drawVolume(volume, DISPLAY_WIDTH - 4, 4, 2, 120);

    for(;;) {
        switch (input_waitForButtonPress()) {
            case BUTTON_CENTER:
                record(buffer);
                drawBuffer(buffer, 4, 4, DISPLAY_WIDTH - 10, 120);
                break;
            case BUTTON_LEFT:
                clear(buffer);
                break;
            case BUTTON_RIGHT:
                play(buffer, volumes[volume]);
                break;
            case BUTTON_UP:
                volume = min(9, volume + 1);
                drawVolume(volume, DISPLAY_WIDTH - 4, 4, 2, 120);
                break;
            case BUTTON_DOWN:
                volume = max(0, volume - 1);
                drawVolume(volume, DISPLAY_WIDTH - 4, 4, 2, 120);
                break;
        }
    }

    return 0;
}

//////////////
// Includes //
//////////////

#include <string.h>
#include <lpc24xx.h>
#include <lcd_grph.h>
#include <font5x7.h>

///////////////////////
// Const Definitions //
///////////////////////

// Used to make it clear when a literal should be interpreted as a bool
// by anyone reading the code.
#define TRUE 1
#define FALSE 0

// Approximate number of iterations of the delay loop per millisecond.
#define DELAY_MULT 3000

// The number of possible buttons that can be pressed.
#define BUTTON_COUNT 5

// Bit numbers in FIO0PIN for each button.
#define BUTTON_NONE -1
#define BUTTON_UP 10
#define BUTTON_DOWN 11
#define BUTTON_LEFT 12
#define BUTTON_RIGHT 13
#define BUTTON_CENTER 22

// Width of each character when drawn to the display, in pixels.
#define CHAR_WIDTH 6
#define BIG_CHAR_WIDTH 12

// Height of each character when drawn to the display, in pixels.
#define CHAR_HEIGHT 8
#define BIG_CHAR_HEIGHT 16

// The number of seconds to record a sample for.
#define SAMPLE_PERIOD 8

// The number of samples recorded per second.
#define SAMPLE_RATE 8000

// The total number of samples to be recorded.
#define SAMPLE_LENGTH (SAMPLE_RATE * SAMPLE_PERIOD)

// Range for the playback timing loop.
#define PLAYBACK_LOOP_ITERS (2600000 / SAMPLE_RATE)

///////////////////////
// Macro Definitions //
///////////////////////

// Produces a binary number with the given number of 1s in a row.
#define bitMask(c) ((1 << (c)) - 1)

// Isolates and shifts a single bit for easy comparison.
#define getBit(x, i) (((x) >> (i)) & 1)

// Isolates and shifts a group of bits for easy comparison.
#define getBits(x, i, c) (((x) >> (i)) & bitMask((c)))
 
// Sets a single bit at the specified position to 0.
#define clrBit(x, i) ((x) & ~(1 << (i)))

// Sets a single bit at the specified position to 1.
#define setBit(x, i) ((x) | (1 << (i)))

// Sets a single bit at the specified position to whatever.
#define cpyBit(x, i, v) (clrBit(x, i) | (((v) & 1) << i))
 
// Sets a group of bits at the specified position to 0.
#define clrBits(x, i, c) ((x) & ~(bitMask(c) << (i)))

// Sets a group of bits at the specified position to 1.
#define setBits(x, i, c) ((x) | (bitMask(c) << (i)))

// Sets a group of bits at the specified position to whatever.
#define cpyBits(x, i, c, v) (clrBits(x, i, c) | (((v) & bitMask(c)) << (i)))

// Compares and gives the smallest of the two inputs.
#define min(a, b) ((a) <= (b) ? (a) : (b))

// Compares and gives the largest of the two inputs.
#define max(a, b) ((a) >= (b) ? (a) : (b))

// Finds the magnitude of a number.
#define abs(a) ((a) < 0 ? -(a) : (a))

//////////////////////
// Type Definitions //
//////////////////////

// I'm homesick for sane languages.
typedef int bool;

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

void wait(int millis);

int input_getButtonPress(void);
int input_waitForButtonPress(void);

bool lcd_charSample(unsigned char ch, int x, int y);
bool lcd_bigCharSample(unsigned char ch, int x, int y);

bool lcd_putBigChar(unsigned short x, unsigned short y, char chr);
void lcd_putBigString(unsigned short x, unsigned short y, char *pStr);

Size lcd_getStringSize(char* str);
Rect lcd_putStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str);

Size lcd_getBigStringSize(char* str);
Rect lcd_putBigStringCentered(unsigned short x, unsigned short y,
    unsigned short w, unsigned short h, char* str);

void adc_init(void);
short adc_read(void);

void dac_init(void);

void clear(char* buffer);
void record(char* buffer);
void play(char* buffer, int volume);

void drawBuffer(char* buffer, int x, int y, int w, int h);
void drawVolume(int volume, int x, int y, int w, int h);

//////////////////////////
// Function Definitions //
//////////////////////////

// Blocks execution for approximately the given number of milliseconds.
void wait(int millis)
{
    volatile int i = 0;
    for (i = 0; i < millis * DELAY_MULT; ++i);
}

// Checks to see if a button has been pressed since the last time this function
// was called, and returns that button's ID if one has. If no button has been
// pressed, returns BUTTON_NONE.
inline int input_getButtonPress(void)
{
    int i, curr, diff;

    // Record the previous state of ~FIO0PIN between invocations.
    static int prev = 0;

    // List of button IDs to loop through.
    const int buttons[BUTTON_COUNT] = {
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_CENTER
    };

    // Invert FIO0PIN so a 1 to signifies a button being pressed.
    curr = ~FIO0PIN;

    // Find the buttons that have been pressed since last invocation.
    diff = (curr ^ prev) & curr;

    // Remember the current button state for next time.
    prev = curr;

    // Find the first button that has just been pressed and return it.
    for (i = 0; i < BUTTON_COUNT; ++i) {
        if (getBit(diff, buttons[i])) return buttons[i];
    }

    // Otherwise, nothing new has been pressed.
    return BUTTON_NONE;
}

// Blocks until a button is pressed, and then returns that buttons's ID.
int input_waitForButtonPress(void)
{
    int btn;
    
    // Super condensed.
    while ((btn = input_getButtonPress()) == BUTTON_NONE);
    return btn;
}

// Finds whether the given texel of the specified character is
// solid in that character's bitmap.
bool lcd_charSample(unsigned char ch, int x, int y)
{
    static unsigned char const font_mask[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };

    // Check to see if the texel is out of bounds.
    if (x < 0 || x >= CHAR_WIDTH) return FALSE;
    if (y < 0 || y >= CHAR_HEIGHT) return FALSE;

    return font5x7[ch][y] & font_mask[x];
}

// Finds whether the given texel of the specified character is
// solid in a 2x super sampled version of that character's bitmap.
bool lcd_bigCharSample(unsigned char ch, int x, int y)
{
    int ix, iy, l, r, t, b, i, j, n; bool dx, dy;

    // Find the texel in the original bitmap.
    ix = x >> 1;
    iy = y >> 1;

    // Find which edge of the original texel this texel is.
    dx = x & 1;
    dy = y & 1;

    // Find bounds for finding neighbours of the texel.
    l = ix - !dx; r = ix + dx; t = iy - !dy; b = iy + dy;

    // Count how many neighbouring texels are solid.
    n = 0;
    for (i = l; i <= r; ++i) {
        for (j = t; j <= b; ++j) {
            if (!lcd_charSample(ch, i, j)) continue;
            else if (i == ix && j == iy) n += 3;
            else if (i == ix || j == iy) n += 2;
            else                         n += 1;
        }
    }

    // Only draw this texel as solid if at least 4 neighbours are.
    return lcd_charSample(ch, ix, iy) || (n >= 4);
}

// Draws the given character at the specified location in a font twice the
// size of the default style. Adapted from lcd_grph.h
bool lcd_putBigChar(unsigned short x, unsigned short y, char chr)
{
    unsigned char i = 0, j = 0;
    unsigned char ch = (unsigned char) chr & 0x7f;
    lcd_color_t color;

    // Don't draw off-screen.
    if((x >= (DISPLAY_WIDTH - BIG_CHAR_WIDTH)) || (y >= (DISPLAY_HEIGHT - BIG_CHAR_HEIGHT))) return FALSE;

    // Treat strange characters as just spaces.
    if((ch < 0x20) || (ch > 0x7f)) ch = 0x20;

    // The character bitmap array is offset by 0x20 from the actual char value.
    ch -= 0x20;


    // Sample each pixel to be drawn. 
    for (i = 0; i < BIG_CHAR_HEIGHT; ++i) {
        for (j = 0; j < BIG_CHAR_WIDTH; ++j) {
            color = lcd_bigCharSample(ch, j, i) ? WHITE : BLACK;
            lcd_point(x, y, color);
            ++x;
        }   
        ++y;
        x -= BIG_CHAR_WIDTH;
    }

    // I'm not sure why lcd_putChar returns TRUE, but we may as well replicate
    // its functionality here just in case.
    return TRUE;
}

// Draws a string in the specified location in a font twice the size of the
// default style.
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

// Calculates the width and height of the given string if it were to be drawn
// on the LCD display in a font twice the default size.
Size lcd_getBigStringSize(char* str)
{
    Size size;

    size = lcd_getStringSize(str);

    size.width *= 2;
    size.height *= 2;

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

// Draws the given string in large lettering in the centre of the provided
// rectangle, as defined by its top-left position, width, and height. Returns
// a Rect structure containing the position and size of the drawn text.
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

// Prepares the Analogue to Digital Converter for usage with the sample
// rate as defined by the SAMPLE_RATE constant.
void adc_init(void)
{
    PINSEL1 = cpyBits(PINSEL1, 16, 2, 1);
    PCONP = setBit(PCONP, 12);

    AD0CR = setBit(cpyBits(2, 8, 8, 12000000 / (SAMPLE_RATE * 16)), 21);
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

void clear(unsigned char* buffer)
{
    int i;

    for (i = 0; i < SAMPLE_LENGTH; ++i) {
        buffer[i] = 0;
    }
}

void record(unsigned char* buffer)
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
    lcd_fillRect(4, 126, DISPLAY_WIDTH - 6, 128, RED);
    lcd_fillRect(4, 4, DISPLAY_WIDTH - 6, 126, BLACK);
}

void play(unsigned char* buffer, int volume)
{
    int b; int val; volatile int i;

    for (b = 0; b < SAMPLE_LENGTH; ++b) {
        val = (((int) buffer[b] << 1) * volume) / 256;
        DACR = cpyBits(0, 6, 10, val);
        for (i = 0; i < PLAYBACK_LOOP_ITERS; ++ i);
    }
}

void drawBuffer(unsigned char* buffer, int x, int y, int w, int h)
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
    unsigned char buffer[SAMPLE_LENGTH]; int volume;

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

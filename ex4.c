
#include "utils.h"
#include "lcd.h"
#include "input.h"

#define WAVE_TRIANGLE 0
#define WAVE_SQUARE 1
#define WAVE_SINE 2
#define WAVE_LAST 2

#define ELEM_NONE 0
#define ELEM_WAVEFORM 1
#define ELEM_KEYBOARD 2
#define ELEM_BOTH (ELEM_WAVEFORM | ELEM_KEYBOARD)

#define KEY_WHITE 0
#define KEY_BLACK 1

#define KEY_WHITE_WIDTH 120
#define KEY_WHITE_HEIGHT 20
#define KEY_BLACK_WIDTH 80
#define KEY_BLACK_HEIGHT 10

#define NOTE_C 0
#define NOTE_C_SHARP 1
#define NOTE_D 2
#define NOTE_D_SHARP 3
#define NOTE_E 4
#define NOTE_F 5
#define NOTE_F_SHARP 6
#define NOTE_G 7
#define NOTE_G_SHARP 8
#define NOTE_A 9
#define NOTE_A_SHARP 10
#define NOTE_B 11

#define OCTAVE_MIN 0
#define OCTAVE_MAX 7

double sin(double theta);

void* malloc(int size);
void* realloc(void* ptr, int size);
void free(void* ptr);

int getWaveFormLength(int hz);
int getHertz(int octave, int note);

int buildWaveForm(short** dest, int type, int octave, int note);

void drawWaveForm(int x, int y, int w, int h, int octave, short* waveForm, int length);
void drawKeyboard(int x, int y, int curNote);
void drawNoteText(int x, int y, int w, int h, int octave, int note);

void redraw(short* waveForm, int length, int octave, int note, int elems);

int getWaveFormLength(int hz) {
	return 266980 / hz;
}

int getHertz(int octave, int note) {
	const int notes[12] = {
		2093, // NOTE_C
		2217, // NOTE_C_SHARP
		2349, // NOTE_D
		2489, // NOTE_D_SHARP
		2637, // NOTE_E
		2794, // NOTE_F
		2960, // NOTE_F_SHARP
		3136, // NOTE_G
		3322, // NOTE_G_SHARP
		3520, // NOTE_A
		3729, // NOTE_A_SHARP
		3951  // NOTE_B
	};

	return notes[note] >> (7 - octave);
}

int buildWaveForm(short** dest, int type, int octave, int note) {
	int length, hz, i; double t, v;

	v = 0.0;

	hz = getHertz(octave, note);
	length = getWaveFormLength(hz);

	if (*dest == NULL) {
		*dest = (short*) malloc(length * sizeof(short));
	} else {
		*dest = (short*) realloc(*dest, length * sizeof(short));
	}

	for (i = 0; i < length; ++i) {
		t = (double) i / (double) length;
		switch (type) {
			case WAVE_SQUARE:
				v = t < 0.5 ? 0.0 : 1.0;
				break;
			case WAVE_TRIANGLE:
				v = t < 0.5 ? t * 2.0 : 2.0 - t * 2.0;
				break;
			case WAVE_SINE:
				v = sin(t * PI * 2.0) * 0.5 + 0.5;
				break;
		}

		(*dest)[i] = (short) (v * 512.0) & 0x3ff;
	}

	return length;
}

void drawWaveForm(int x, int y, int w, int h, int octave, short* waveForm, int length)
{
	int i, p, c, l;

	lcd_fillRect(x, y, x + w, y + h, BLACK);
	lcd_drawRect(x - 1, y - 1, x + w + 1, y + h + 1, WHITE);

	l = (waveForm[0] * (w - 8)) / 0x200;
	for (i = 1; i < h; ++ i) {
		p = (i * (1 << (8 - octave))) % length;
		c = (waveForm[p] * (w - 8)) / 0x200;
		lcd_line(x + l + 4, y + i - 1, x + c + 4, y + i, RED);
		l = c;
	}
}

void drawKeyboard(int x, int y, int curNote)
{
	int xPos, yPos, key;

	const bool keyTypes[] = {
		KEY_WHITE,
		KEY_BLACK,
		KEY_WHITE,
		KEY_BLACK,
		KEY_WHITE,
		KEY_WHITE,
		KEY_BLACK,
		KEY_WHITE,
		KEY_BLACK,
		KEY_WHITE,
		KEY_BLACK,
		KEY_WHITE
	};

	xPos = x;
	yPos = y;
	for (key = NOTE_C; key <= NOTE_B; ++key) {
		if (keyTypes[key] == KEY_BLACK) continue;

		lcd_fillRect(xPos, yPos + 1, xPos + KEY_WHITE_WIDTH - 1,
			yPos + KEY_WHITE_HEIGHT - 1, key == curNote ? RED : WHITE);

		yPos += KEY_WHITE_HEIGHT;
	}

	xPos = x + KEY_WHITE_WIDTH - KEY_BLACK_WIDTH;
	yPos = y - (KEY_BLACK_HEIGHT >> 1);
	for (key = NOTE_C; key <= NOTE_B; ++key) {
		if (keyTypes[key] == KEY_WHITE) {
			yPos += KEY_WHITE_HEIGHT;
			continue;
		}

		lcd_fillRect(xPos, yPos, xPos + KEY_BLACK_WIDTH, yPos + KEY_BLACK_HEIGHT,
			key == curNote ? RED : BLACK);
	}
}

void drawNoteText(int x, int y, int w, int h, int octave, int note)
{
	static char* noteStrs[] = {
		" C ",
		" C# ",
		" D ",
		" D# ",
		" E ",
		" F ",
		" F# ",
		" G ",
		" G# ",
		" A ",
		" A# ",
		" B "
	};

	char octStr[2] = { (char) (48 + octave), '\0' };

	lcd_putStringCentered(x, y, w, h / 2, noteStrs[note]);
	lcd_putStringCentered(x, y + h / 2, w, h / 2, octStr);
}

void redraw(short* waveForm, int length, int octave, int note, int elems)
{
	if (elems & ELEM_WAVEFORM) {
		drawWaveForm(4, 4, DISPLAY_WIDTH - 24 - KEY_WHITE_WIDTH,
			DISPLAY_HEIGHT - 8, octave, waveForm, length);
	}

	if (elems & ELEM_KEYBOARD) {
		drawKeyboard(DISPLAY_WIDTH - 8 - KEY_WHITE_WIDTH, 8, note);
		drawNoteText(DISPLAY_WIDTH - 8 - KEY_WHITE_WIDTH, DISPLAY_HEIGHT / 2 - 4,
			KEY_WHITE_WIDTH, DISPLAY_HEIGHT / 2 - 8, octave, note);
	}
}

int main(void)
{
	bool playing; short* waveForm; int octave, note, type, length; volatile int i;

	lcd_init();

	waveForm = NULL;

	PINSEL1 = cpyBits(PINSEL1, 20, 2, 1 << 1);

	octave = 4;
	note = NOTE_C;

	type = WAVE_TRIANGLE;
	length = buildWaveForm(&waveForm, type, octave, note);

	playing = TRUE;

	redraw(waveForm, length, octave, note, ELEM_BOTH);

	for (i = 0;; i = i + 1 < length ? i + 1 : 0) {
		switch (input_getButtonPress()) {
			case BUTTON_CENTER:
				playing = !playing;
				break;
			case BUTTON_DOWN:
				++note;
				if (note > NOTE_B) {
					note = NOTE_C;
					octave = min(octave + 1, OCTAVE_MAX);
				}
				length = buildWaveForm(&waveForm, type, octave, note);
				redraw(waveForm, length, octave, note, ELEM_BOTH);
				break;
			case BUTTON_UP:
				--note;
				if (note < NOTE_C) {
					note = NOTE_B;
					octave = max(octave - 1, OCTAVE_MIN);
				}
				length = buildWaveForm(&waveForm, type, octave, note);
				redraw(waveForm, length, octave, note, ELEM_BOTH);
				break;
			case BUTTON_LEFT:
				--type;
				if (type < 0) type = WAVE_LAST;
				length = buildWaveForm(&waveForm, type, octave, note);
				redraw(waveForm, length, octave, note, ELEM_WAVEFORM);
				break;
			case BUTTON_RIGHT:
				++type;
				if (type > WAVE_LAST) type = 0;
				length = buildWaveForm(&waveForm, type, octave, note);
				redraw(waveForm, length, octave, note, ELEM_WAVEFORM);
				break;
			default:
				if (playing) DACR = cpyBits(0, 6, 10, waveForm[i]);
				else DACR = cpyBits(0, 6, 10, 0x100);
				break;
		}
	}

	free(waveForm);

	return 0;
}

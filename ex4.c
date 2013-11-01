
#include "utils.h"
#include "input.h"

#define WAVE_TRIANGLE 0
#define WAVE_SQUARE 1
#define WAVE_SINE 2
#define WAVE_LAST 2

#define NOTE_A 0
#define NOTE_A_SHARP 1
#define NOTE_B 2
#define NOTE_C 3
#define NOTE_C_SHARP 4
#define NOTE_D 5
#define NOTE_D_SHARP 6
#define NOTE_E 7
#define NOTE_F 8
#define NOTE_F_SHARP 9
#define NOTE_G 10
#define NOTE_G_SHARP 11

int buildWaveForm(short** dest, int type, int hz);

int getWaveFormLength(int hz) {
	return 250000 / hz;
}

int buildWaveForm(short** dest, int type, int hz) {
	int length, i; double t; double v;

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

		(*dest)[i] = (int) (v * 512.0) & 0x3ff;
	}

	return length;
}

int main(void)
{
	bool playing; short* waveForm; int note, type, length; volatile int i;

	const int notes[12] = {
		220, // NOTE_A
		233, // NOTE_A_SHARP
		247, // NOTE_B
		262, // NOTE_C
		277, // NOTE_C_SHARP
		294, // NOTE_D
		311, // NOTE_D_SHARP
		330, // NOTE_E
		349, // NOTE_F
		370, // NOTE_F_SHARP
		392, // NOTE_G
		415  // NOTE_G_SHARP
	};

	PINSEL1 = cpyBits(PINSEL1, 20, 2, 1 << 1);

	note = NOTE_A;
	type = WAVE_TRIANGLE;
	length = buildWaveForm(&waveForm, type, notes[note]);

	playing = TRUE;

	for (i = 0;; i = i + 1 < length ? i + 1 : 0) {
		switch (input_getButtonPress()) {
			case BUTTON_CENTER:
				playing = !playing;
				break;
			case BUTTON_UP:
				++note;
				if (note > NOTE_G_SHARP) note = NOTE_A;
				length = buildWaveForm(&waveForm, type, notes[note]);
				break;
			case BUTTON_DOWN:
				--note;
				if (note < NOTE_A) note = NOTE_G_SHARP;
				length = buildWaveForm(&waveForm, type, notes[note]);
				break;
			case BUTTON_LEFT:
				--type;
				if (type < 0) type = WAVE_LAST;
				length = buildWaveForm(&waveForm, type, notes[note]);
				break;
			case BUTTON_RIGHT:
				++type;
				if (type > WAVE_LAST) type = 0;
				length = buildWaveForm(&waveForm, type, notes[note]);
				break;
		}

		if (playing) DACR = cpyBits(0, 6, 10, waveForm[i]);
		else DACR = cpyBits(0, 6, 10, 0x100);
	}

	free(waveForm);

	return 0;
}

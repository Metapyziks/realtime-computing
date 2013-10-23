#include <lpc24xx.h>
#include "utils.h"

#define KEYPOINTS 18
#define SPEEDS 8

typedef struct {
	double hz;
	int mr2;
} KeyPoint;

KeyPoint keyPoints[KEYPOINTS] = {
	{ 0.000000000,      0 },
	{ 39.68253968,	 5000 },
	{ 50.00000000,	 6000 },
	{ 60.24096386,	 7000 },
	{ 63.69426752,	 7500 },
	{ 66.66666667,	 8000 },
	{ 72.99270073,	 9000 },
	{ 75.18796992,	10000 },
	{ 84.45945946,	12000 },
	{ 90.57971014,	15000 },
	{ 98.03921569,	18000 },
	{ 98.42519685,	20000 },
	{ 104.1666667,	22500 },
	{ 106.3829787,	25000 },
	{ 108.6956522,	27500 },
	{ 109.6491228,	30000 },
	{ 112.6126126,	35000 },
	{ 115.7407407,	40000 }
};

int findMR2Val(double hz) {
	int i;
	KeyPoint curr, prev;
	double t;

	prev = keyPoints[0];
	for (i = 1; i < KEYPOINTS; ++i) {
		curr = keyPoints[i];

		if (hz < curr.hz) {
			t = (hz - prev.hz) / (curr.hz - prev.hz);
			return (int) round(t * curr.mr2 + (1 - t) * prev.mr2);
		}

		prev = curr;
	}

	return keyPoints[KEYPOINTS - 1].mr2;
}

void setMotorSpeed(double hz)
{
	char* str;

	PWM0MR0 = 40000;
	PWM0MR2 = findMR2Val(hz);

	str = toString(PWM0MR2);
	
	lcd_fillScreen(BLACK);
	lcd_putStringCentered(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, str);

	free(str);

	PWM0LER = 1 << 2;
}

void initMotor()
{
	PINSEL2 = setBits(PINSEL2, 6, 2);
	PWM0PCR = setBit(PWM0PCR, 10);

	setMotorSpeed(0);

	PWM0TCR = setBit(setBit(0, 0), 3);
}

int main()
{
	int i;
	double speeds[SPEEDS];

	int curSpeed = 0;

	for (i = 0; i < SPEEDS; ++i) {
		speeds[i] = 42.0 * sqrt(i);
	}

	lcd_init();
	initMotor();

	for (;;) {
		switch (getButtonPress()) {
			case BUTTON_LEFT:
				curSpeed = (curSpeed + SPEEDS - 1) % SPEEDS;
				setMotorSpeed(speeds[curSpeed]);
				break;
			case BUTTON_RIGHT:
				curSpeed = (curSpeed + 1) % SPEEDS;
				setMotorSpeed(speeds[curSpeed]);
				break;
			case BUTTON_UP:
				setMotorSpeed(speeds[curSpeed]);
				break;
			case BUTTON_DOWN:
				setMotorSpeed(0);
				break;
		}
	}

	return 0;
}

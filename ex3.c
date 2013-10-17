#include <lpc24xx.h>
#include "utils.h"

void setMotorSpeed(int hz)
{
	PWM0MR0 = 400000;
	PWM0MR2 = 100000;
}

void initMotor()
{
	PINSEL2 = setBits(PINSEL2, 6, 2);
	PWM0PCR = setBit(PWM0PCR, 10);
}

void startMotor()
{
	PWM0TCR = setBit(setBit(0, 0), 3);
}

int main()
{
	initMotor();
	setMotorSpeed(1);
	// startMotor();
}

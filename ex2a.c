#include "utils.h"

int main()
{
	int i = 0;

	FIO3DIR = 0xFFFFFFFF;

	for (;;) {
		wait(250);
		FIO3PIN = cpyBit(0, 16, i++ & 1);
	}

	return 0;
}

#include <lpc24xx.h>
#include "utils.h"

int main()
{
	int i = 0;

	FIO3DIR = 0xFFFFFFFF;

	for (;;) {
		wait(500);
		FIO3PIN = setBit(0, i++ & 31);
	}

	return 0;
}

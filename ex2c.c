#include "utils.h"

int main()
{
	int someVar = 824719;

	FIO3DIR = 0xFFFFFFFF;
	FIO3PIN = 0xFFFFFFFF & someVar;

	return 0;
}

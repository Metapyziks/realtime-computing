#ifndef UTILS_H_GUARD
#define UTILS_H_GUARD

#include <string.h>
#include <lpc24xx.h>

#define TRUE 1
#define FALSE 0

#define DELAY_MULT 3000

#define E 2.71828182845904523536028747135266249775724709369995

#define bitMask(c) ((1 << (c)) - 1)
 
#define getBit(x, i) (((x) >> (i)) & 1)
#define getBits(x, i, c) (((x) >> (i)) & bitMask((c)))
 
#define clrBit(x, i) ((x) & ~(1 << (i)))
#define setBit(x, i) ((x) | (1 << (i)))
#define cpyBit(x, i, v) (clrBit(x, i) | (((v) & 1) << i))
 
#define clrBits(x, i, c) ((x) & ~(bitMask(c) << (i)))
#define setBits(x, i, c) ((x) | (bitMask(c) << (i)))
#define cpyBits(x, i, c, v) (clrBits(x, i, c) | (((v) & bitMask(c)) << (i)))

#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

typedef int bool;

float randFloat()
{
    return (rand() % 65536) / 65536f;
}

void wait(int millis)
{
	volatile int i = 0;
	for (i = 0; i < millis * DELAY_MULT; ++i);
}

#endif

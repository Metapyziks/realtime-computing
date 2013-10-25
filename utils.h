/**
 * File Name  : utils.h
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : Some general definitions, and a few helper functions that have
 *				no specific context.
 */

#ifndef UTILS_H_GUARD
#define UTILS_H_GUARD

//////////////
// Includes //
//////////////

#include <string.h>
#include <lpc24xx.h>

///////////////////////
// Const Definitions //
///////////////////////

// I'm pretty sure this is the case.
#define TRUE 1
#define FALSE 0

// Approximate number of iterations of the delay loop per millisecond.
#define DELAY_MULT 3000

// According to Wikipedia.
#define PI 3.14159265358979323846264338327950288419716939937511
#define E 2.71828182845904523536028747135266249775724709369995

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

// Gives the absolute value of the input.
#define abs(a) ((a) < 0 ? -(a) : (a))

//////////////////////
// Type Definitions //
//////////////////////

// I'm homesick for sane languages.
typedef int bool;

///////////////////////////
// Function Declarations //
///////////////////////////

void srand(int seed);
int rand(void);

double round(double val);
double sqrt(double val);

float randFloat(void);
void wait(int millis);

//////////////////////////
// Function Definitions //
//////////////////////////

// Returns a random single precision number between 0.0 and 1.0.
float randFloat(void)
{
    return (rand() % 65536) / 65536.0f;
}

// Blocks execution for approximately the given number of milliseconds.
void wait(int millis)
{
	volatile int i = 0;
	for (i = 0; i < millis * DELAY_MULT; ++i);
}

#endif

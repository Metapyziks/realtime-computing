/**
 * File Name  : motor.h
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : A method to initialize the motor, and another to set the motor
 *              frequency in hertz by finding an interpolated MR2 value from a
 *              a list of pre-recorded 'keypoints'.
 */

#ifndef MOTOR_H_GUARD
#define MOTOR_H_GUARD

//////////////
// Includes //
//////////////

#include "utils.h"

///////////////////////
// Const Definitions //
///////////////////////

// The number of recorded keypoints.
#define KEYPOINTS 18

//////////////////////
// Type Definitions //
//////////////////////

// Contains a frequency / match register 2 value pair from a recorded keypoint.
typedef struct {
    double hz;
    int mr2;
} motor_KeyPoint;

//////////////////////////
// Function Definitions //
//////////////////////////

// For a given frequency (in hertz), find the corresponding value for the MR2
// register that would produce (approximately) that motor speed.
int motor_findMR2Val(double hz) {
    int i; motor_KeyPoint curr, prev; double t;

    // List of all recorded keypoints in order of frequency.
    const motor_KeyPoint _keyPoints[KEYPOINTS] = {
        { 0.000000000,     0 },
        { 39.68253968,  5000 },
        { 50.00000000,  6000 },
        { 60.24096386,  7000 },
        { 63.69426752,  7500 },
        { 66.66666667,  8000 },
        { 72.99270073,  9000 },
        { 75.18796992, 10000 },
        { 84.45945946, 12000 },
        { 90.57971014, 15000 },
        { 98.03921569, 18000 },
        { 98.42519685, 20000 },
        { 104.1666667, 22500 },
        { 106.3829787, 25000 },
        { 108.6956522, 27500 },
        { 109.6491228, 30000 },
        { 112.6126126, 35000 },
        { 115.7407407, 40000 }
    };

    // Record the previous keypoint (starting with the first) to be used when
    // interpolating the final MR2 value.
    prev = _keyPoints[0];

    // Loop through each keypoint after the first until one with a frequency
    // greater than the desired value is found.
    for (i = 1; i < KEYPOINTS; ++i) {
        curr = _keyPoints[i];

        // If this keypoint exceeds the desired frequency, find an interpolated
        // MR2 between this keypoint and the previous one.
        if (curr.hz >= hz) {
            t = (hz - prev.hz) / (curr.hz - prev.hz);
            return (int) round(t * curr.mr2 + (1 - t) * prev.mr2);
        }

        // Record the current keypoint to be used as the previous one next
        // iteration.
        prev = curr;
    }

    return _keyPoints[KEYPOINTS - 1].mr2;
}

// Set up the various motor registers and stuff.
void motor_init()
{
    // Apparently this enables PWM output on P1.3 or something.
    PINSEL2 = setBits(PINSEL2, 6, 2);

    // This tells the PWM unit to control something or other.
    PWM0PCR = setBit(PWM0PCR, 10);

    // Use 40k for the pulse period counter.
    PWM0MR0 = 40000;

    // Don't start the motor just yet.
    PWM0MR2 = 0;

    // Start the PWM unit.
    PWM0TCR = setBit(setBit(0, 0), 3);
}

// Set the motor to spin at a specified speed in revolutions per second.
void motor_setSpeed(double hz)
{
    PWM0MR2 = motor_findMR2Val(hz);

    // Update next cycle
    PWM0LER = 1 << 2;
}

#endif

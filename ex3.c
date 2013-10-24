/**
 * File Name  : ex3.c
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : Exercise 3 main source file. A 3D star field with the viewer
 *              flying through it at controllable speeds, using the motor to
 *              provide authentic space ship engine sound effects.
 * Controls   : Up button to accelerate, down button to slow down, and centre
 *              button to warp.
 */

//////////////
// Includes //
//////////////

#include "lcd.h"
#include "motor.h"
#include "input.h"

///////////////////////
// Const Definitions //
///////////////////////

// The total number of stars to display.
#define STAR_COUNT 128

// The distance of the virtual view plane from the camera, which affects the
// field of view of the scene.
#define Z_PLANE_DIST 0.5f

// The acceleration / deceleration rate while boosting.
#define BOOST_ACCEL 1.01f

// The acceleration / deceleration rate while manually changing camera speed.
#define MANUAL_ACCEL 1.2f

// The maximum speed the camera is allowed to travel at.
#define MAX_SPEED 0.0625f

// The minimum speed the camera is allowed to travel at.
#define MIN_SPEED 0.00390625f

//////////////////////
// Type Definitions //
//////////////////////

// Star structure, recording the location of the star in 3D space.
typedef struct {
    float x;
    float y;
    float z;
} Star;

//////////////////////
// Global Variables //
//////////////////////

// The current speed of the camera.
float speed = MIN_SPEED;

// An interpolated version of the camera speed for the HUD bars.
float smoothSpeed = MIN_SPEED;

//////////////////////////
// Function Definitions //
//////////////////////////

// Converts the given speed into a value from 0.0 to 1.0, where 0.0 is
// MIN_SPEED and 1.0 is MAX_SPEED. Assumes the given speed is within those
// two bounds.
float getSpeedRatio(float speedVal)
{
	return (float) sqrt((speedVal - MIN_SPEED) / (MAX_SPEED - MIN_SPEED));
}

// Sets the speed of the motor to reflect the current camera speed.
void updateMotor()
{
	motor_setSpeed(getSpeedRatio(speedVal) * 120.0);
}

// Accelerate the camera by the given amount, and update the motor speed.
// Returns TRUE if the camera is now at maximum speed, and FALSE otherwise.
bool accelerate(float accel)
{
	speed *= accel;

    // Not so fast!
	if (speed >= MAX_SPEED) {
        speed = MAX_SPEED;

        updateMotor();
        return TRUE;
    }

	updateMotor();
    return FALSE;
}

// Decelerate the camera by the given amount, and update the motor speed.
// Returns TRUE if the camera is now at minimum speed, and FALSE otherwise.
bool decelerate(float accel)
{
	speed /= accel;

    // You're going too slow...
	if (speed <= MIN_SPEED) {
        speed = MIN_SPEED;

        updateMotor();
        return TRUE;
    }

	updateMotor();
    return FALSE;
}

// Sets the X and Y components of a given star's position to random values
// between -0.5 and 0.5.
void randomizeStar(Star* star)
{
    star->x = randFloat() - 0.5f;
    star->y = randFloat() - 0.5f;
}

// Projects a given star in 3D, and draws it to the screen. The star is drawn
// as a line, with a length proportional to the camera speed to give the
// illusion of non-instantaneous camera exposure. Draws the star in the
// provided colour.
void renderStar(Star star, int colour)
{
    float mn, mf; int xn, yn, xf, yf;

    // Don't draw a star if it is behind the camera! This should never occur
    // anyway, since we push them back when they pass the camera.
    if (star.z <= 0) return;

    // Work out the perspective multipliers for the near and far points of the
    // line we will draw for the star. Also ensures that we don't draw nothing
    // if the camera is stopped.
    mn = Z_PLANE_DIST / star.z;
    mf = Z_PLANE_DIST / (star.z + max(speed, MIN_SPEED));

    // Using the perspective multipliers, calculate the two (X, Y) coordinate
    // pairs for the star's line. Positions the line to be relative to the
    // centre of the screen, and stretches it.
    xn = (int) (star.x * mn * DISPLAY_WIDTH) + (DISPLAY_WIDTH >> 1);
    yn = (int) (star.y * mn * DISPLAY_HEIGHT) + (DISPLAY_HEIGHT >> 1);
    xf = (int) (star.x * mf * DISPLAY_WIDTH) + (DISPLAY_WIDTH >> 1);
    yf = (int) (star.y * mf * DISPLAY_HEIGHT) + (DISPLAY_HEIGHT >> 1);

    // Draw the line!
    lcd_line(xn, yn, xf, yf, colour);
}

// Draw a box with a certain percentage filled up that represents the current
// camera speed at the given position, and with the given size.
void renderSpeedBar(int x, int y, int width, int height)
{
	float ratio; int unfilledHeight;

    // Find how much of the box to fill.
	ratio = 1f - getSpeedRatio(smoothSpeed);
    unfilledHeight = (int) (ratio * (height - 1));

    // First draw the whole box in white, from which the non-filled-in area
    // will be carved out by drawing a smaller black box.
    lcd_fillRect(x, y, x + width, y + height, WHITE);
    lcd_fillRect(x + 1, y + 1, x + width - 1, y + unfilledHeight, BLACK);
}

// Entry point, containing initialization and the main draw / update loop.
int main()
{
    int i;

    // Array of all existing stars.
    Star stars[STAR_COUNT];

    // Records whether boost / warp effect is active.
    bool boosting = FALSE;

    // Records whether boost is currently accelerating or decelerating.
    bool accelerating = FALSE;

    // Seed the RNG with a carefully constructed non-arbitrary number.
    srand(0x3ae14c92);

    // Prepare the LCD display and motor for use.
    lcd_init();
    motor_init();

    // Give each star a random starting position.
    for (i = 0; i < STAR_COUNT; ++i) {
        randomizeStar(&stars[i]);
        stars[i].z = randFloat();
    }

    // Main loop.
    for (;;) {
        // Erase all the stars from the sky. I'm assuming it's faster to do
        // this than do lcd_fillScreen(BLACK).
        for (i = 0; i < STAR_COUNT; ++i) {
            renderStar(stars[i], BLACK);
        }

        // If we aren't currently warping, accept button press inputs.
        if (!boosting) {
            switch (input_getButtonPress()) {
                case BUTTON_UP:
                    accelerate(MANUAL_ACCEL);
                    break;
                case BUTTON_DOWN:
                    decelerate(MANUAL_ACCEL);
                    break;
                case BUTTON_CENTER:
                    boosting = TRUE;
                    accelerating = TRUE;
                    break;
            }

        // Otherwise, if we are in the acceleration phase of warping, speed up
        // the camera until it is at maximum speed.
        } else if (accelerating && accelerate(BOOST_ACCEL)) {
            accelerating = FALSE;

        // Otherwise, if we are in the deceleration phase of warping, slow down
        // the camera until it is at minimum speed.
        } else if (!accelerating && decelerate(BOOST_ACCEL)) {
            boosting = FALSE;
        }

        // Now loop through each star again, to update their positions and draw
        // them to the display.
        for (i = 0; i < STAR_COUNT; ++i) {
            stars[i].z -= speed;

            // If the star is behind the camera, push it to the back of the
            // scene and randomize its X and Y position.
            if (stars[i].z <= 0) {
                stars[i].z = 1f;
                randomizeStar(&stars[i]);
            }

            // Draw the star in white.
            renderStar(stars[i], WHITE);
        }

        // Ease smoothSpeed towards the current value of speed.
        smoothSpeed += (speed - smoothSpeed) * .1f;

        // Draw the speed bar things on either side of the display.
        renderSpeedBar(4, 4, 6, DISPLAY_HEIGHT - 8);
        renderSpeedBar(DISPLAY_WIDTH - 10, 4, 6, DISPLAY_HEIGHT - 8);

        // I think we have some time to spare.
        wait(16);
    }

    // This will never happen.
    return 0;
}

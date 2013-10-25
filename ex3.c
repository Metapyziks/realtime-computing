/**
 * File Name  : ex3.c
 * Author     : James King (gvnj58)
 * Email      : james.king3@durham.ac.uk
 * Contents   : Exercise 3 main source file. A 3D star field with the viewer
 *              flying through it at controllable speeds, using the motor to
 *              provide authentic space ship engine sound effects.
 * Controls   : Up button to accelerate, down button to slow down, left and
                right to strafe, and center to toggle warp mode (auto ramp up).
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
#define Z_PLANE_DIST 0.75f

// The acceleration / deceleration rate while boosting.
#define BOOST_ACCEL 1.01f

// The number of frames to keep at full speed while boosting.
#define BOOST_FRAMES 60

// The acceleration / deceleration rate while manually changing camera speed.
#define MANUAL_ACCEL 1.01f

// The maximum speed the camera is allowed to travel at.
#define MAX_SPEED 0.0625f

// The minimum speed the camera is allowed to travel at.
#define MIN_SPEED 0.00390625f

//////////////////////
// Type Definitions //
//////////////////////

// Star structure, recording the location of the star in 3D space, and colour.
typedef struct {
    float x;
    float y;
    float z;
    int clr;
} Star;

///////////////////////////
// Function Declarations //
///////////////////////////

float getSpeedRatio(float speedVal);

void updateMotor(float speed);

bool accelerate(float* speed, float accel);
bool decelerate(float* speed, float accel);

void randomizeStar(Star* star);

void renderStar(Star star, float speed, int colour);
void renderSpeedBar(int x, int y, int width, int height,
    float speed, int colour);

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
void updateMotor(float speed)
{
    float ratio;

    ratio = getSpeedRatio(speed);

    if (ratio == 0.0f) {
        motor_setSpeed(0.0);
    } else {
        motor_setSpeed(ratio * 100.0 + 20.0);
    }
}

// Accelerate the camera by the given amount, and update the motor speed.
// Returns TRUE if the camera is now at maximum speed, and FALSE otherwise.
bool accelerate(float* speed, float accel)
{
	*speed *= accel;

    // Not so fast!
	if (*speed >= MAX_SPEED) {
        *speed = MAX_SPEED;

        updateMotor(*speed);
        return TRUE;
    }

	updateMotor(*speed);
    return FALSE;
}

// Decelerate the camera by the given amount, and update the motor speed.
// Returns TRUE if the camera is now at minimum speed, and FALSE otherwise.
bool decelerate(float* speed, float accel)
{
	*speed /= accel;

    // You're going too slow...
	if (*speed <= MIN_SPEED) {
        *speed = MIN_SPEED;

        updateMotor(*speed);
        return TRUE;
    }

	updateMotor(*speed);
    return FALSE;
}

// Sets the X and Y components of a given star's position to random values
// between -0.5 and 0.5.
void randomizeStar(Star* star)
{
    const int colours[4] = {
        WHITE,
        LIGHT_GRAY,
        CYAN,
        YELLOW
    };

    star->x = randFloat() - 0.5f;
    star->y = randFloat() - 0.5f;
    star->clr = colours[(int) (randFloat() * randFloat() * 4)];
}

// Projects a given star in 3D, and draws it to the screen. The star is drawn
// as a line, with a length proportional to the camera speed to give the
// illusion of non-instantaneous camera exposure. Draws the star in the
// provided colour.
void renderStar(Star star, float speed, int colour)
{
    float mn, mf; int xn, yn, xf, yf;

    // Don't draw a star if it is behind the camera! This should never occur
    // anyway, since we push them back when they pass the camera.
    if (star.z <= 0.0f) return;

    // Work out the perspective multipliers for the near and far points of the
    // line we will draw for the star. Also ensures that we don't draw nothing
    // if the camera is stopped.
    mn = Z_PLANE_DIST / star.z;
    mf = Z_PLANE_DIST / (star.z + max(speed * 2.0f, MIN_SPEED));

    // Make sure the line doesn't go off screen because apparently that crashes
    // the program occasionally.
    mn = min(mn, 0.5f / abs(star.x));
    mn = min(mn, 0.5f / abs(star.y));

    // Using the perspective multipliers, calculate the two (X, Y) coordinate
    // pairs for the star's line. Positions the line to be relative to the
    // centre of the screen, and stretches it.
    xn = (int) ((star.x * mn + 0.5f) * DISPLAY_WIDTH);
    yn = (int) ((star.y * mn + 0.5f) * DISPLAY_HEIGHT);
    xf = (int) ((star.x * mf + 0.5f) * DISPLAY_WIDTH);
    yf = (int) ((star.y * mf + 0.5f) * DISPLAY_HEIGHT);

    // If the line isn't completely off-screen, draw it.
    if (xf >= 0 && xf < DISPLAY_WIDTH && yf >= 0 && yf < DISPLAY_HEIGHT) {
        lcd_line(xn, yn, xf, yf, colour);
    }
}

// Draw a box with a certain percentage filled up that represents the current
// camera speed at the given position, and with the given size.
void renderSpeedBar(int x, int y, int width, int height,
    float speed, int colour)
{
	float ratio; int filled;

    // Find how much of the box to fill.
	ratio = 1.0f - getSpeedRatio(speed);
    filled = (int) round(ratio * (height - 2));

    // Draw a white outline around the bar.
    lcd_drawRect(x, y, x + width, y + height, colour);

    // If the bar isn't full, draw a black box to erase any part of the bar
    // left from last frame that shouldn't be there.
    if (filled > 0) {
        lcd_fillRect(x + 2, y + 2, x + width - 2, y + filled, BLACK);
    }

    // If the bar isn't empty, draw a white box for the bar.
    if (filled < height - 3) {
        lcd_fillRect(x + 2, y + 2 + filled,
            x + width - 2, y + height - 2, colour);
    }
}

// Entry point, containing initialization and the main draw / update loop.
int main(void)
{
    int i;

    // Array of all existing stars.
    Star stars[STAR_COUNT];

    // Records whether boost / warp effect is active.
    bool boosting = FALSE;

    // Records whether boost is currently accelerating or decelerating.
    bool accelerating = TRUE;

    // The number of frames the camera has been boosting at full speed for.
    int boostFrames = 0;

    // The current speed of the camera.
    float speed = MIN_SPEED;

    // Lateral camera speed.
    float strafeSpeed = 0.0f;

    // An interpolated version of the camera speed for the HUD bars.
    float smoothSpeed = MIN_SPEED;

    // Seed the RNG with a carefully constructed non-arbitrary number.
    srand(0x3ae14c92);

    // Prepare the LCD display and motor for use.
    lcd_init();
    motor_init();

    // Make sure the motor is going at the initial speed.
    updateMotor(speed);

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
            renderStar(stars[i], speed, BLACK);
        }

        strafeSpeed *= 0.95f;

        // Strafe left when left button is pressed.
        if (input_isKeyDown(BUTTON_LEFT)) {
            strafeSpeed -= 0.001f;
        }

        // Likewise, but for strafing right.
        if (input_isKeyDown(BUTTON_RIGHT)) {
            strafeSpeed += 0.001f;
        }
            
        // If the centre key has been pressed, toggle warping.
        if (input_getButtonPress() == BUTTON_CENTER) {
            boosting = !boosting;
        }

        // If we aren't currently warping, accept button press inputs.
        if (!boosting) {
            // Accelerate when pressing up.
            if (input_isKeyDown(BUTTON_UP)) {
                accelerate(&speed, MANUAL_ACCEL);
            }

            // Decelerate when pressing down.
            if (input_isKeyDown(BUTTON_DOWN)) {
                decelerate(&speed, MANUAL_ACCEL);
            }

        // Otherwise, if we are in the acceleration phase of warping, speed up
        // the camera until it is at maximum speed.
        } else if (accelerating && accelerate(&speed, BOOST_ACCEL)) {

            // When we've been at maximum speed for the given duration, start
            // to decelerate.
            if (++boostFrames >= BOOST_FRAMES) {
                boostFrames = 0;
                accelerating = FALSE;
            }

        // Otherwise, if we are in the deceleration phase of warping, slow down
        // the camera until it is at minimum speed.
        } else if (!accelerating && decelerate(&speed, BOOST_ACCEL)) {
            
            // When we've been at minimum speed for the given duration, start
            // to accelerate.
            if (++boostFrames >= BOOST_FRAMES) {
                boostFrames = 0;
                accelerating = TRUE;
            }
        }

        // Now loop through each star again, to update their positions and draw
        // them to the display.
        for (i = 0; i < STAR_COUNT; ++i) {
            stars[i].z -= speed;
            stars[i].x -= strafeSpeed;

            // If the star is too far to the left, move it right.
            if (stars[i].x < -0.5f) {
                stars[i].x += 1.0f;
            } 

            // If the star is too far to the right, move it left.
            if (stars[i].x >= 0.5f) {
                stars[i].x -= 1.0f;
            }

            // If the star is behind the camera, push it to the back of the
            // scene and randomize its X and Y position.
            if (stars[i].z <= 0.0f) {
                stars[i].z = 1.0f;
                randomizeStar(&stars[i]);
            }

            // Draw the star in white.
            renderStar(stars[i], speed, stars[i].clr);
        }

        // Ease smoothSpeed towards the current value of speed.
        smoothSpeed += (speed - smoothSpeed) * 0.1f;

        // Draw the speed bar things on either side of the display.
        renderSpeedBar(4, 4, 6, DISPLAY_HEIGHT - 8,
            smoothSpeed, boosting ? YELLOW : WHITE);
        renderSpeedBar(DISPLAY_WIDTH - 10, 4, 6, DISPLAY_HEIGHT - 8,
            smoothSpeed, boosting ? YELLOW : WHITE);

        // I think we have some time to spare.
        wait(16);
    }

    // This should never happen.
    return (int) (1.0 / 0.0);
}

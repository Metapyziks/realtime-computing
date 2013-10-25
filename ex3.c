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

#include <lpc24xx.h>
#include <lcd_grph.h>

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

// The number of recorded motor speed keypoints.
#define KEYPOINT_COUNT 18

// The number of possible buttons that can be pressed.
#define BUTTON_COUNT 5

// Bit numbers in FIO0PIN for each button.
#define BUTTON_NONE -1
#define BUTTON_UP 10
#define BUTTON_DOWN 11
#define BUTTON_LEFT 12
#define BUTTON_RIGHT 13
#define BUTTON_CENTER 22

// The total number of stars to display.
#define STAR_COUNT 128

// The distance of the virtual view plane from the camera, which affects the
// field of view of the scene.
#define Z_PLANE_DIST 0.75f

// The acceleration / deceleration rate while warping.
#define BOOST_ACCEL 1.01f

// The number of frames to keep at full speed while warping.
#define BOOST_FRAMES 60

// The acceleration / deceleration rate while manually changing camera speed.
#define MANUAL_ACCEL 1.01f

// The maximum speed the camera is allowed to travel at.
#define MAX_SPEED 0.0625f

// The minimum speed the camera is allowed to travel at.
#define MIN_SPEED 0.00390625f

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

// I'm homesick for C#.
typedef int bool;

// Contains a frequency / match register 2 value pair from a recorded keypoint.
typedef struct {
    double hz;
    int mr2;
} motor_KeyPoint;

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

void srand(int seed);
int rand(void);

double round(double val);
double sqrt(double val);

float randFloat(void);
void wait(int millis);

int motor_findMR2Val(double hz);
void motor_init(void);
void motor_setSpeed(double hz);

int input_getButtonPress(void);
bool input_isKeyDown(int key);

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

// For a given frequency (in hertz), find the corresponding value for the MR2
// register that would produce (approximately) that motor speed.
int motor_findMR2Val(double hz) {
    int i; motor_KeyPoint curr, prev; double t;

    // List of all recorded keypoints in order of frequency.
    const motor_KeyPoint _keyPoints[KEYPOINT_COUNT] = {
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
    for (i = 1; i < KEYPOINT_COUNT; ++i) {
        curr = _keyPoints[i];

        // If this keypoint exceeds the desired frequency, find an interpolated
        // MR2 between this keypoint and the previous one.
        if (curr.hz >= hz) {
            t = (hz - prev.hz) / (curr.hz - prev.hz);
            return (int) round(t * curr.mr2 + (1.0 - t) * prev.mr2);
        }

        // Record the current keypoint to be used as the previous one next
        // iteration.
        prev = curr;
    }

    return _keyPoints[KEYPOINT_COUNT - 1].mr2;
}

// Set up the various motor registers and stuff.
void motor_init(void)
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

// Checks to see if a button has been pressed since the last time this function
// was called, and returns that button's ID if one has. If no button has been
// pressed, returns BUTTON_NONE.
int input_getButtonPress(void)
{
    int i, curr, diff;

    // Record the previous state of ~FIO0PIN between invocations.
    static int prev = 0;

    // List of button IDs to loop through.
    const int buttons[BUTTON_COUNT] = {
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_CENTER
    };

    // Why on earth does FIO0PIN use a 0 to signify a button being pressed?
    curr = ~FIO0PIN;

    // Find the buttons that have been pressed since last invocation.
    diff = (curr ^ prev) & curr;

    // Remember the current button state for next time.
    prev = curr;

    // Find the first button that has just been pressed and return it.
    for (i = 0; i < BUTTON_COUNT; ++i) {
        if (getBit(diff, buttons[i])) return buttons[i];
    }

    // Otherwise, nothing new has been pressed.
    return BUTTON_NONE;
}

// Checks to see if the specified button is currently pressed.
bool input_isKeyDown(int button)
{
    return getBit(~FIO0PIN, button);
}

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

    // Records whether warp / warp effect is active.
    bool warping = FALSE;

    // Records whether warp is currently accelerating or decelerating.
    bool accelerating = TRUE;

    // The number of frames the camera has been warping at full speed for.
    int warpFrames = 0;

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
            warping = !warping;
        }

        // If we aren't currently warping, accept button press inputs.
        if (!warping) {
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
            if (++warpFrames >= BOOST_FRAMES) {
                warpFrames = 0;
                accelerating = FALSE;
            }

        // Otherwise, if we are in the deceleration phase of warping, slow down
        // the camera until it is at minimum speed.
        } else if (!accelerating && decelerate(&speed, BOOST_ACCEL)) {
            
            // When we've been at minimum speed for the given duration, start
            // to accelerate.
            if (++warpFrames >= BOOST_FRAMES) {
                warpFrames = 0;
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
            smoothSpeed, warping ? YELLOW : WHITE);
        renderSpeedBar(DISPLAY_WIDTH - 10, 4, 6, DISPLAY_HEIGHT - 8,
            smoothSpeed, warping ? YELLOW : WHITE);

        // I think we have some time to spare.
        wait(16);
    }

    // This should never happen.
    return (int) (1.0 / 0.0);
}

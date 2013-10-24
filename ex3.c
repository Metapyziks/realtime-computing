#include "lcd.h"
#include "motor.h"
#include "input.h"

#define STAR_COUNT 128
#define Z_PLANE_DIST 0.5f

#define BOOST_ACCEL 1.01f
#define MANUAL_ACCEL 1.2f

#define MAX_SPEED 0.0625f
#define MIN_SPEED 0.00390625f

typedef struct {
    float x;
    float y;
    float z;
} Star;

Star stars[STAR_COUNT];

int boosting = FALSE;
int accelerating = FALSE;

float speed = MIN_SPEED;
float smoothSpeed = MIN_SPEED;

float getSpeedRatio(float speedVal)
{
	return (float) sqrt((speedVal - MIN_SPEED) / (MAX_SPEED - MIN_SPEED));
}

void updateMotor()
{
	motor_setSpeed(getSpeedRatio(speed) * 120.0);
}

int accelerate(float accel)
{
	speed *= accel;

	if (speed >= MAX_SPEED) {
        speed = MAX_SPEED;

        updateMotor();
        return TRUE;
    }

    updateMotor();
    return FALSE;
}

int decelerate(float accel)
{
	speed /= accel;

	if (speed <= MIN_SPEED) {
        speed = MIN_SPEED;

        updateMotor();
        return TRUE;
    }

    updateMotor();
    return FALSE;
}

float randFloat()
{
    return (rand() % 65536) / 65536f;
}

void randomizeStar(Star* star)
{
    star->x = randFloat() - 0.5f;
    star->y = randFloat() - 0.5f;
}

void renderStar(Star star)
{
    float mn, mf;
    int xn, yn, xf, yf;

    if (star.z <= 0) return;

    mn = Z_PLANE_DIST / star.z;
    mf = Z_PLANE_DIST / (star.z + max(speed, MIN_SPEED));

    xn = (int) (star.x * mn * DISPLAY_WIDTH) + (DISPLAY_WIDTH >> 1);
    yn = (int) (star.y * mn * DISPLAY_HEIGHT) + (DISPLAY_HEIGHT >> 1);
    xf = (int) (star.x * mf * DISPLAY_WIDTH) + (DISPLAY_WIDTH >> 1);
    yf = (int) (star.y * mf * DISPLAY_HEIGHT) + (DISPLAY_HEIGHT >> 1);

    lcd_line(xn, yn, xf, yf, WHITE);
}

void renderHUD(int x, int y, int width, int height)
{
	float ratio;

	ratio = 1f - getSpeedRatio(smoothSpeed);

    lcd_drawRect(x, y, x + width, y + height, WHITE);
    lcd_fillRect(x, y + (int) (ratio * height), x + width, y + height, WHITE);
}

void init()
{
    int i;

    srand(0x3ae14c92);

	lcd_init();
    motor_init();

    for (i = 0; i < STAR_COUNT; ++i) {
        randomizeStar(&stars[i]);
        stars[i].z = randFloat();
    }
}

void think()
{
    int i;

    smoothSpeed += (speed - smoothSpeed) * .1f;

    if (!boosting) {
        switch (input_getButtonPress()) {
            case BUTTON_UP:
                accelerate(MANUAL_ACCEL); break;
            case BUTTON_DOWN:
                decelerate(MANUAL_ACCEL); break;
            case BUTTON_CENTER:
                boosting = TRUE;
                accelerating = TRUE;
                break;
        }
    } else if (accelerating) {
        if (accelerate(BOOST_ACCEL)) {
        	accelerating = FALSE;
        }
    } else {
        if (decelerate(BOOST_ACCEL)) {
        	boosting = FALSE;
        }
    }

    for (i = 0; i < STAR_COUNT; ++i) {
        stars[i].z -= speed;

        if (stars[i].z <= 0) {
            randomizeStar(&stars[i]);
            stars[i].z = 1f;
        }
    }
}

void render()
{
    int i;

    for (i = 0; i < STAR_COUNT; ++i) {
        renderStar(stars[i]);
    }

    renderHUD(4, 4, 6, DISPLAY_HEIGHT - 8);
    renderHUD(DISPLAY_WIDTH - 10, 4, 6, DISPLAY_HEIGHT - 8);
}

int main()
{
    init();

    for (;;) {
        think();
        render();
        wait(16);
    }

    return 0;
}

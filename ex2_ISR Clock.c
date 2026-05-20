#include <stdio.h>
#include "NuMicro.h"
#include "tmr.h"
#include "system_init.h"
#include "GUI.h"
#include "display.h"

/* define */
#define MaxSpeed    50000
#define MinSpeed    1000

#define SW_UP       PC9
#define SW_DOWN     PG4
#define SW_CTR      PG3

#pragma anon_unions

/* global variable define */
uint32_t SpeedCtl;
uint32_t timecount;
uint32_t sec = 0;
uint32_t min = 0;
uint32_t hour = 0;

void Clock_Task(void);
void clock_init(void);
void clock_tick(void);
void LED_showing(uint32_t SpeedCtl);
void GPIO_init(void);
uint32_t JoyStick(unsigned char BIN_state);

typedef union
{
    struct
    {
        unsigned UP   : 1;
        unsigned DOWN : 1;
        unsigned CTR  : 1;
    };

    unsigned char JState;

} Joystick_union;

Joystick_union JSUnion;

int main(void)
{
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Init TMR0 for timecount */
    TMR0_Initial();

    /* Set Initialization time */
    clock_init();

    /* Set GPIO */
    GPIO_init();

    /* Open GUI display */
    Display_Init();

    while (1)
    {
        char clock_buf[20];
        char speed_buf[20];
        uint32_t speed;

        JSUnion.UP = SW_UP;
        JSUnion.DOWN = SW_DOWN;
        JSUnion.CTR = SW_CTR;

        SpeedCtl = JoyStick(JSUnion.JState);

        speed = 51000 - SpeedCtl;

        LED_showing(speed);

        clock_tick();

        sprintf(clock_buf, "%02d:%02d:%02d", hour, min, sec);
        sprintf(speed_buf, "speed = %.1f (s)", speed / 10000.0);

        Display_buf(clock_buf, 270, 1);
        Display_buf(speed_buf, 1, 1);
    }
}

/* GPIO initialize */
void GPIO_init(void)
{
    GPIO_SetMode(PA, BIT0, GPIO_MODE_INPUT);      // SW1

    GPIO_SetMode(PH, BIT6, GPIO_MODE_OUTPUT);     // LEDR1
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT);     // LEDG1

    GPIO_SetMode(PC, BIT9, GPIO_MODE_INPUT);      // Joystick UP
    GPIO_SetMode(PG, BIT4, GPIO_MODE_INPUT);      // Joystick DOWN
    GPIO_SetMode(PG, BIT3, GPIO_MODE_INPUT);      // Joystick CENTER
}

/* time initialize */
void clock_init(void)
{
    sec = 0;
    min = 0;
    hour = 0;
}

void clock_tick(void)
{
    static uint32_t old_timecount = 0;

    if ((uint32_t)(timecount - old_timecount) < 10000)
        return;

    old_timecount = timecount;

    sec++;

    if (sec == 60)
    {
        sec = 0;
        min++;

        if (min == 60)
        {
            min = 0;
            hour++;

            if (hour == 24)
                hour = 0;
        }
    }
}

uint32_t JoyStick(unsigned char BIN_state)
{
    static uint32_t old_timecount = 0;
    static uint32_t SpeedCtl = 41000;

    if ((uint32_t)(timecount - old_timecount) < 1000)
        return SpeedCtl;

    old_timecount = timecount;

    switch (BIN_state)
    {
        case 0x06:
            // speed up
            if (SpeedCtl < MaxSpeed)
                SpeedCtl += 1000;
            break;

        case 0x05:
            // speed down
            if (SpeedCtl > MinSpeed)
                SpeedCtl -= 1000;
            break;

        case 0x03:
            // reset
            SpeedCtl = 51000;
            break;

        default:
            break;
    }

    return SpeedCtl;
}

/* LED Toggle */
void LED_showing(uint32_t SpeedCtl)
{
    static uint32_t old_timecount = 0;
    static unsigned char f = 0x01;

    if ((uint32_t)(timecount - old_timecount) < SpeedCtl)
        return;

    f ^= 0x01;

    PH6 = (f) ? 1 : 0;
    PH7 = (f) ? 0 : 1;

    old_timecount = timecount;
}
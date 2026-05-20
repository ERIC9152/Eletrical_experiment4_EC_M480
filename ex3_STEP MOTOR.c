#include <stdio.h>
#include "NuMicro.h"
#include "tmr.h"
#include "system_init.h"
#include "GUI.h"
#include "display.h"
#include "BNCTL.h"
#include "StepMotorAgent.h"

/* define max and min speed */
#define MaxSpeed    17
#define MinSpeed    1

/* global variable define */
uint32_t timecount;
uint8_t dir;
uint32_t speed;

int main(void)
{
    char motor_state_buf[30];
    char SPD_buf[30];
    char direction_buf[30];
    uint32_t speedCTL;

    SYS_Init();

    BIN_init();

    StepMtr_Initial();

    Display_Init();

    TMR0_Initial();

    dir = 1;
    speed = 0;

    while (1)
    {
        BIN_task();

        if (Btn_IsOneShot(0x01) == 0x01)
        {
            // stop
            speed = 0;
            GUI_Clear();
            Btn_OneShotClear(0x01);
        }

        if (Btn_IsOneShot(0x02) == 0x02)
        {
            // change direction
            dir ^= 0x01;
            GUI_Clear();
            Btn_OneShotClear(0x02);
        }

        if (Btn_IsOneShot(0x04) == 0x04)
        {
            // speed up
            if (speed < MaxSpeed)
                speed++;
            else
                speed = MaxSpeed;

            GUI_Clear();
            Btn_OneShotClear(0x04);
        }

        if (Btn_IsOneShot(0x08) == 0x08)
        {
            // speed down
            if (speed > MinSpeed)
                speed--;
            else
                speed = MinSpeed;

            GUI_Clear();
            Btn_OneShotClear(0x08);
        }

        if (speed)
            speedCTL = 1000 / speed;
        else
            speedCTL = 0;

        StepMtr_Task(dir, speedCTL);

        if (speed == 0)
            sprintf(motor_state_buf, "motor state: stop");
        else
            sprintf(motor_state_buf, "motor state: run");

        sprintf(SPD_buf, "speed : %02d rpm", speed * 6);

        if (dir == 1)
            sprintf(direction_buf, "dir: clockwise");
        else
            sprintf(direction_buf, "dir: counterclockwise");

        Display_buf(motor_state_buf, 1, 1);
        Display_buf(SPD_buf, 1, 25);
        Display_buf(direction_buf, 1, 49);
    }
}
#include <stdio.h>
#include "NuMicro.h"
#include "ADCAgent.h"
#include "TempSensor.h"
#include "system_init.h"
#include "display.h"
#include "tmr.h"
#include "GUI.h"
#include "sys.h"
#include "BNCTL.h"
#include "StepMotorAgent.h"

#define MaxSpeed    17
#define MinSpeed    1

uint32_t timecount = 0;
uint32_t speed;
uint8_t dir;
uint8_t speed_ctl_mode;

int main(void)
{
    char ADC_value_buf[20];
    char M487sensor_temp_value_buf[30];
    char thermistor_temp_value_buf[30];
    char speed_buf[20];
    char speed_ctl_mode_buf[20];

    uint32_t speedCTL;

    SYS_Init();

    Temp_Sensor_Enable();

    TMR0_Initial();

    Display_Init();

    ADC_Initial();

    BIN_init();

    StepMtr_Initial();

    dir = 1;
    speed = 10;
    speed_ctl_mode = 0x00;

    while (1)
    {
        BIN_task();

        if (Btn_IsOneShot(0x01) == 0x01)
        {
            speed = 0;

            GUI_Clear();
            Btn_OneShotClear(0x01);
        }

        if (Btn_IsOneShot(0x02) == 0x02)
        {
            if (Btn_IsDown(0x01) == 0x01)
            {
                speed_ctl_mode ^= 0x01;
            }
            else
            {
                dir ^= 0x01;
            }

            GUI_Clear();
            Btn_OneShotClear(0x02);
        }

        if (Btn_IsOneShot(0x04) == 0x04)
        {
            if (speed < MaxSpeed)
                speed++;
            else
                speed = MaxSpeed;

            GUI_Clear();
            Btn_OneShotClear(0x04);
        }

        if (Btn_IsOneShot(0x08) == 0x08)
        {
            if (speed > MinSpeed)
                speed--;
            else
                speed = MinSpeed;

            GUI_Clear();
            Btn_OneShotClear(0x08);
        }

        /*
           speed_ctl_mode = 0：使用按鈕控制速度
           speed_ctl_mode = 1：使用 ADC VR 控制速度
        */
        if (speed_ctl_mode == 0x00)
        {
            if (speed)
                speedCTL = 1000 / speed;
            else
                speedCTL = 0;
        }
        else
        {
            speedCTL = ADC_GetVR() * 8;
        }

        sprintf(ADC_value_buf, "ADC value : %03d", ADC_GetVR());
        Display_buf(ADC_value_buf, 1, 1);

        sprintf(M487sensor_temp_value_buf, "M487sensor_temp : %2.1f",
                ADC_GetM487Temperature());
        Display_buf(M487sensor_temp_value_buf, 1, 40);

        sprintf(thermistor_temp_value_buf, "ThermistorTemp : %d",
                ADC_ConvThermistorTempToReal());
        Display_buf(thermistor_temp_value_buf, 1, 79);

        sprintf(speed_buf, "Speed : %02d rpm", speed * 6);
        Display_buf(speed_buf, 1, 118);

        sprintf(speed_ctl_mode_buf, "Mode : %01d", speed_ctl_mode);
        Display_buf(speed_ctl_mode_buf, 1, 157);

        StepMtr_Task(dir, speedCTL);

        ADC_Task();
    }
}
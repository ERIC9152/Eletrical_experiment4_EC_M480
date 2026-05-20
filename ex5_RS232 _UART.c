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
#include "UART1.h"
#include <stdio.h>

/* define max and min speed */
#define MaxSpeed    10
#define MinSpeed    1

/* global variable define */
uint32_t timecount = 0;
uint8_t speed;
uint8_t dir;

char c;
char sendbuf[100];
unsigned int baudrate;
char baudrate_buf[20];

void Select_mode(void);
void BTN_speed_control(void);
void ADC_speed_control(void);
void UART1_speed_control(void);

int main(void)
{
    char ADC_value_buf[20];
    char M487sensor_temp_value_buf[30];
    char thermistor_temp_value_buf[30];
    char speed_buf[20];
    char mode_buf[20];
    char receive_buf[20];

    uint8_t mode = 0;
    uint8_t btn_pressed_once = 0;

    SYS_Init();

    Temp_Sensor_Enable();

    TMR0_Initial();

    Display_Init();

    ADC_Initial();

    BIN_init();

    UART1_Initial();

    StepMtr_Initial();

    dir = 1;
    speed = 5;
    baudrate = 115200;

    while (1)
    {
        /*
           同時按下 BTN1 與 BTN2 時切換模式：
           mode 0：Button 控制
           mode 1：ADC 控制
           mode 2：UART 控制
        */
        if (Btn_IsDown(0x01) && Btn_IsDown(0x02) && btn_pressed_once == 0)
        {
            mode = (mode == 2) ? 0 : mode + 1;
            btn_pressed_once = 1;
            GUI_Clear();
        }
        else if (!(Btn_IsDown(0x01) && Btn_IsDown(0x02)))
        {
            btn_pressed_once = 0;
        }

        switch (mode)
        {
            case 0:
                BTN_speed_control();
                break;

            case 1:
                ADC_speed_control();
                break;

            case 2:
                UART1_speed_control();
                break;

            default:
                BTN_speed_control();
                break;
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

        sprintf(mode_buf, "Mode = %d", mode);
        Display_buf(mode_buf, 1, 157);

        sprintf(baudrate_buf, "baudrate: %d", baudrate);
        Display_buf(baudrate_buf, 130, 196);

        sprintf(receive_buf, "received: %c", c);
        Display_buf(receive_buf, 1, 196);

        StepMtr_Task(dir, speed);

        ADC_Task();

        BIN_task();
    }
}

void UART1_speed_control(void)
{
    if (UART1_IsRxDataReady())
    {
        c = UART1_ReadByte();

        GUI_Clear();

        switch (c)
        {
            case '+':
                if (speed == MaxSpeed)
                {
                    StrPush("Max speed\r\n");
                }
                else
                {
                    speed++;
                    StrPush("Speed Up\r\n");
                }
                break;

            case '-':
                if (speed == MinSpeed || speed == 0)
                {
                    StrPush("Min speed\r\n");
                }
                else
                {
                    speed--;
                    StrPush("Speed down\r\n");
                }
                break;

            case 's':
                speed = 0;
                StrPush("Stop\r\n");
                break;

            case 'r':
                dir ^= 0x01;
                StrPush("Reverse\r\n");
                break;

            case 'p':
                sprintf(sendbuf,
                        "Speed: %d rpm: %d Direction: %s\r\n",
                        speed,
                        speed * 6,
                        (dir ? "clockwise" : "counterclockwise"));
                StrPush(sendbuf);
                break;

            case 'i':
                baudrate = 9600;
                ChangeBaudRate(baudrate);
                StrPush("Baudrate changed to 9600\r\n");
                break;

            default:
                StrPush("Unknown\r\n");
                break;
        }

        UART1_TxTask();
    }
}

void BTN_speed_control(void)
{
    if (Btn_IsOneShot(0x01) == 0x01)
    {
        speed = 0;

        GUI_Clear();
        Btn_OneShotClear(0x01);
    }

    if (Btn_IsOneShot(0x02) == 0x02)
    {
        dir ^= 0x01;

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
}

void ADC_speed_control(void)
{
    uint8_t v;

    v = ADC_GetVR();

    if (v <= 30)
    {
        speed = 2;
    }
    else if (v > 30 && v <= 60)
    {
        speed = 5;
    }
    else if (v > 60 && v <= 90)
    {
        speed = 8;
    }
    else
    {
        speed = 10;
    }
}
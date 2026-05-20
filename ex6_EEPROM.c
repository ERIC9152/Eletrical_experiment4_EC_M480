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
#include "I2C_EEPROM.h"
#include <stdio.h>
#include <string.h>

/* define max and min speed */
#define MaxSpeed    10
#define MinSpeed    1

/* global variable define */
uint32_t timecount;

uint8_t BTN_speed;
uint8_t ADC_speed;
uint8_t UART_speed;
uint8_t speed;
uint8_t dir;

uint8_t sec;
uint8_t min;
uint8_t hour;

uint8_t BAUD_DIV_LOW;
uint8_t BAUD_DIV_HIGH;

uint8_t CMDlen;
uint8_t CMDstate;

uint16_t BAUD_DIV;
uint32_t baudrate;

char c;
char sendbuf[100];
char time_buf[100];
char baudrate_buf[20];
char CMD[20];

char *BAUDCMD = "BAUD=";

unsigned int CMD_NUM;

void Select_mode(void);
void BTN_speed_control(void);
void ADC_speed_control(void);
void UART1_speed_control(void);
void EEPROM_control(void);
void SaveDataToEEPROM(void);
void ReadDataFromEEPROM(void);
int ConfigWithEEPROM(void);
void SaveAge(void);
void ClearEEPROM(void);
void ClockTick_Aging(void);

#pragma anon_unions

typedef union
{
    struct
    {
        uint8_t CHECK;
        uint32_t BR;
        uint8_t SPEED;
        uint8_t DIR;
        uint8_t MIN;
    };

    uint8_t DATA[10];

} EEPROM_table;

EEPROM_table eepromData;

int main(void)
{
    char ADC_value_buf[20];
    char M487sensor_temp_value_buf[30];
    char thermistor_temp_value_buf[30];
    char speed_buf[20];
    char mode_buf[20];
    char receive_buf[20];

    uint8_t mode = 0;

    SYS_Init();

    Temp_Sensor_Enable();

    TMR0_Initial();

    Display_Init();

    ADC_Initial();

    BIN_init();

    UART1_Initial();

    StepMtr_Initial();

    I2C_EEPROM_Init();

    if (!ConfigWithEEPROM())
    {
        baudrate = 115200;
        ChangeBaudRate(baudrate);

        UART_speed = 5;
        BTN_speed = 5;
        ADC_speed = 5;

        dir = 1;
        min = 0;
    }

    CMDlen = 0;
    CMDstate = 0;

    hour = 0;
    sec = 0;

    while (1)
    {
        if (Btn_IsOneShot(0x02) == 0x02)
        {
            mode = (mode == 3) ? 0 : mode + 1;
            Btn_OneShotClear(0x02);
            GUI_Clear();
        }

        switch (mode)
        {
            case 0:
                BTN_speed_control();
                speed = BTN_speed;
                break;

            case 1:
                ADC_speed_control();
                speed = ADC_speed;
                break;

            case 2:
                UART1_speed_control();
                speed = UART_speed;
                break;

            case 3:
                EEPROM_control();
                speed = UART_speed;
                break;

            default:
                BTN_speed_control();
                speed = BTN_speed;
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

        sprintf(time_buf, "Age: %04d", min);
        Display_buf(time_buf, 196, 1);

        StepMtr_Task(dir, speed);

        ADC_Task();

        BIN_task();

        ClockTick_Aging();
    }
}

void ClearEEPROM(void)
{
    eepromData.CHECK = 0x00;
    I2C_EEPROM_Write(0x0001, eepromData.CHECK);
}

void SaveAge(void)
{
    eepromData.MIN = min;
    I2C_EEPROM_Write(0x000B, eepromData.MIN);
}

int ConfigWithEEPROM(void)
{
    ReadDataFromEEPROM();

    if (eepromData.CHECK != 0xA5)
    {
        return 0;
    }

    baudrate = eepromData.BR;
    UART_speed = eepromData.SPEED;
    dir = eepromData.DIR;
    min = eepromData.MIN;

    ChangeBaudRate(baudrate);

    return 1;
}

void SaveDataToEEPROM(void)
{
    uint16_t i;

    eepromData.CHECK = 0xA5;

    for (i = 0x0001; i < 0x000C; i++)
    {
        I2C_EEPROM_Write(i, eepromData.DATA[i - 1]);
    }
}

void ReadDataFromEEPROM(void)
{
    uint16_t i;

    for (i = 0x0001; i < 0x000C; i++)
    {
        eepromData.DATA[i - 1] = I2C_EEPROM_Read(i);
    }
}

void EEPROM_control(void)
{
    if (Btn_IsOneShot(0x01) == 0x01)
    {
        ClearEEPROM();
        GUI_Clear();
        Btn_OneShotClear(0x01);
    }

    if (Btn_IsOneShot(0x04) == 0x04)
    {
        eepromData.BR = baudrate;
        eepromData.SPEED = UART_speed;
        eepromData.DIR = dir;
        eepromData.MIN = min;

        SaveDataToEEPROM();

        GUI_Clear();
        Btn_OneShotClear(0x04);
    }

    if (Btn_IsOneShot(0x08) == 0x08)
    {
        ReadDataFromEEPROM();

        baudrate = eepromData.BR;
        UART_speed = eepromData.SPEED;
        dir = eepromData.DIR;
        min = eepromData.MIN;

        ChangeBaudRate(baudrate);

        GUI_Clear();
        Btn_OneShotClear(0x08);
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
                if (UART_speed == MaxSpeed)
                {
                    StrPush("Max speed\r\n");
                }
                else
                {
                    UART_speed++;
                    StrPush("Speed Up\r\n");
                }
                break;

            case '-':
                if (UART_speed == MinSpeed || UART_speed == 0)
                {
                    StrPush("Min speed\r\n");
                }
                else
                {
                    UART_speed--;
                    StrPush("Speed Down\r\n");
                }
                break;

            case 's':
                UART_speed = 0;
                StrPush("Stop\r\n");
                break;

            case 'r':
                dir ^= 0x01;
                StrPush("Reverse\r\n");
                break;

            case 'p':
                sprintf(sendbuf,
                        "Speed: %d\r\nrpm: %d\r\nDirection: %s\r\n",
                        UART_speed,
                        UART_speed * 6,
                        (dir ? "Clockwise" : "Counterclockwise"));
                StrPush(sendbuf);
                break;

            default:
                switch (CMDstate)
                {
                    case 0:
                        if (c == BAUDCMD[CMDlen])
                        {
                            CMDlen++;
                        }
                        else
                        {
                            CMDlen = 0;
                            StrPush("Error\r\n");
                        }

                        if (BAUDCMD[CMDlen] == '\0')
                        {
                            CMDlen = 0;
                            CMDstate = 1;
                        }
                        break;

                    case 1:
                        if (c != 0x0D)
                        {
                            CMD[CMDlen++] = c;
                        }
                        else
                        {
                            CMD[CMDlen] = '\0';

                            sscanf(CMD, "%d", &CMD_NUM);

                            sprintf(sendbuf, "Get CMD: BAUD=%d\r\n", CMD_NUM);
                            StrPush(sendbuf);

                            baudrate = CMD_NUM;

                            UART1_TxData();

                            CMDstate = 0;
                            CMDlen = 0;

                            ChangeBaudRate(baudrate);
                        }
                        break;
                }
                break;
        }
    }

    UART1_TxTask();
}

void BTN_speed_control(void)
{
    if (Btn_IsOneShot(0x01) == 0x01)
    {
        BTN_speed = 0;

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
        if (BTN_speed < MaxSpeed)
            BTN_speed++;
        else
            BTN_speed = MaxSpeed;

        GUI_Clear();
        Btn_OneShotClear(0x04);
    }

    if (Btn_IsOneShot(0x08) == 0x08)
    {
        if (BTN_speed > MinSpeed)
            BTN_speed--;
        else
            BTN_speed = MinSpeed;

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
        ADC_speed = 2;
    }
    else if (v > 30 && v <= 60)
    {
        ADC_speed = 5;
    }
    else if (v > 60 && v <= 90)
    {
        ADC_speed = 8;
    }
    else
    {
        ADC_speed = 10;
    }
}

void ClockTick_Aging(void)
{
    static uint32_t old_timecount = 0;

    if ((uint32_t)(timecount - old_timecount) < 10000)
        return;

    old_timecount = timecount;

    sec++;

    if (sec >= 60)
    {
        sec = 0;
        min++;

        SaveAge();

        if (min >= 60)
        {
            min = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
        }
    }
}
#include "M480.h"

int32_t main(void)
{
    // Input Pin set
    GPIO_SetMode(PA, BIT0, GPIO_MODE_INPUT);
    GPIO_SetMode(PA, BIT1, GPIO_MODE_INPUT);

    GPIO_SetMode(PG, BIT2, GPIO_MODE_INPUT);    // UP
    GPIO_SetMode(PC, BIT10, GPIO_MODE_INPUT);   // DOWN
    GPIO_SetMode(PG, BIT3, GPIO_MODE_INPUT);    // CENTER

    // Output Pin set
    GPIO_SetMode(PH, BIT6, GPIO_MODE_OUTPUT);   // LEDR1
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT);   // LEDG1

    while (1)
    {
        if ((PG2 == 0) && (PC10 == 1))           // UP pressed
        {
            PH6 = 1;    // LEDR1 OFF
            PH7 = 0;    // LEDG1 ON
        }
        else if ((PG2 == 1) && (PC10 == 0))      // DOWN pressed
        {
            PH6 = 0;    // LEDR1 ON
            PH7 = 1;    // LEDG1 OFF
        }
        else if (PG3 == 0)                       // CENTER pressed
        {
            PH6 = 0;    // LEDR1 ON
            PH7 = 0;    // LEDG1 ON
        }
        else
        {
            PH6 = 1;    // LEDR1 OFF
            PH7 = 1;    // LEDG1 OFF
        }
    }
}
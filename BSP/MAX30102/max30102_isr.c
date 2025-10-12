/**
 * @file    max30102_isr.c
 * @brief   MAX30102 interrupt service routines
 */

#include "max30102_example.h"
#include "max30102_hal.h"

/* External variables --------------------------------------------------------*/
extern volatile bool data_ready_flag;

/**
 * @brief MAX30102 data ready interrupt handler
 */
void MAX30102_DataReady_ISR(void)
{
    data_ready_flag = true;
}

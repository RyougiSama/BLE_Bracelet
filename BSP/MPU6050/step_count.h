#ifndef __STEP_COUNT_H
#define __STEP_COUNT_H

#include "mpu6050.h"

extern uint16_t g_step;

void Timer_Handler_StepCount(void);

#endif

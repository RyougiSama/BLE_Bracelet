#ifndef __BLOOD_MEASURE_TASK_H
#define __BLOOD_MEASURE_TASK_H

#include "max30102_example.h"

extern int32_t g_heart_rate, g_spo2;
extern bool g_is_hr_valid, g_is_spo2_valid;

void Task_BloodMeasure(void);
void Blood_Data_Update(void);

#endif

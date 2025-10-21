#ifndef __MAX30102_USER_H
#define __MAX30102_USER_H

#include <stdbool.h>
#include <stdint.h>

extern int32_t g_spo2;                     // SPO2 value
extern int8_t g_spo2_valid;                // indicator to show if the SP02 calculation is valid
extern int32_t g_heart_rate;               // heart rate value
extern int8_t g_hr_valid;                  // indicator to show if the heart rate calculation is valid

void MAX30102_System_Init(void);
void Task_BloodMeasure(void);
bool MAX30102_IsVaid(void);
void max30102_test(void);

#endif

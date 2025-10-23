#ifndef __OLED_USER_H
#define __OLED_USER_H

#include "rtc.h"

#define OLED_MAIN_INTERFACE_COUNT 4

typedef enum { OLED_STANDBY = 0, OLED_MAX30102, OLED_STEP_GPS, OLED_TEST } OLED_MainInterface;

extern OLED_MainInterface g_curr_main_interface;
extern RTC_DateTypeDef g_rtc_date;
extern RTC_TimeTypeDef g_rtc_time;

void Task_OLED_Update(void);
void OLED_MoveToNextInterface(void);

#endif

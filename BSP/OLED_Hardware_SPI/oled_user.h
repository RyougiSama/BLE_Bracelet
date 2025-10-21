#ifndef __OLED_USER_H
#define __OLED_USER_H

#define OLED_MAIN_INTERFACE_COUNT 3

typedef enum {
    OLED_STANDBY = 0,
    OLED_MAX30102,
    OLED_STEP_GPS,
    OLED_TEST
} OLED_MainInterface;

extern OLED_MainInterface g_curr_main_interface;

void Task_OLED_Update(void);
void OLED_MoveToNextInterface(void);

#endif

#ifndef __OLED_USER_H
#define __OLED_USER_H

typedef enum {
    OLED_STANDBY = 0,
} OLED_MainInterface;

extern OLED_MainInterface g_curr_main_interface;

void Task_OLED_Upate(void);

#endif

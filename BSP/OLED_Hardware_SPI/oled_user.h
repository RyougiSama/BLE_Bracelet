#ifndef __OLED_USER_H
#define __OLED_USER_H

#define OLED_MAIN_INTERFACE_COUNT 2

typedef enum {
    OLED_STANDBY = 0,
    OLED_TEST
} OLED_MainInterface;

extern OLED_MainInterface g_curr_main_interface;

void Task_OLED_Update(void);
void OLED_MoveToNextInterface(void);

#endif

#include "oled_user.h"

#include "oled_hardware_spi.h"

OLED_MainInterface g_curr_main_interface = OLED_STANDBY;

void Task_OLED_Upate(void)
{
    switch (g_curr_main_interface) {
        case OLED_STANDBY:
            OLED_ShowString(0, 0, (uint8_t *)"< BLE Bracelet >", 16);
            break;
        default:
            break;
    }
}

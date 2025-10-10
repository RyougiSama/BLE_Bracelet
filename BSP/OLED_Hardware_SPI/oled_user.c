#include "oled_user.h"

#include <stdio.h>

#include "oled_hardware_spi.h"
#include "rtc.h"

OLED_MainInterface g_curr_main_interface = OLED_STANDBY;
RTC_DateTypeDef g_rtc_date;
RTC_TimeTypeDef g_rtc_time;

static void OLED_STANDBY_Display(void)
{
    OLED_ShowString(0, 0, (uint8_t *)"< BLE Bracelet >", 16);
    // 读取时间和日期
    read_bkup(&hrtc);
    HAL_RTC_GetDate(&hrtc, &g_rtc_date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &g_rtc_time, RTC_FORMAT_BIN);
    write_bkup(&hrtc);
    // 显示当前日期
    char date_str[16] = {0};
    snprintf(date_str, sizeof(date_str), "Date:%02d/%02d/20%02d", g_rtc_date.Date, g_rtc_date.Month,
             g_rtc_date.Year);
    OLED_ShowString(0, 2, (uint8_t *)date_str, 16);
    // 显示当前时间
    char time_str[16] = {0};
    snprintf(time_str, sizeof(time_str), "Time:%02d:%02d:%02d", g_rtc_time.Hours,
             g_rtc_time.Minutes, g_rtc_time.Seconds);
    OLED_ShowString(0, 4, (uint8_t *)time_str, 16);
}

static void OLED_TEST_Display(void)
{
    OLED_ShowString(0, 0, (uint8_t *)"OLED TEST MODE", 16);
}

void Task_OLED_Upate(void)
{
    switch (g_curr_main_interface) {
        case OLED_STANDBY:
            OLED_STANDBY_Display();
            break;
        case OLED_TEST:
            OLED_TEST_Display();
            break;
        default:
            break;
    }
}

void OLED_MoveToNextInterface(void)
{
    g_curr_main_interface = (OLED_MainInterface)(((uint8_t)g_curr_main_interface + 1) % 2);
    OLED_Clear();
}

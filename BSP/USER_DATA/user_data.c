#include "user_data.h"

#include <stdio.h>

#include "max30102_user.h"
#include "oled_user.h"
#include "rtc.h"

UserHealthData g_newest_user_hr_data;

void UserData_UpdateHealth(void) {
    // 读取时间
    read_bkup(&hrtc);
    HAL_RTC_GetDate(&hrtc, &g_rtc_date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &g_rtc_time, RTC_FORMAT_BIN);
    write_bkup(&hrtc);
    // 更新最新的用户健康数据
    snprintf(g_newest_user_hr_data.time, sizeof(g_newest_user_hr_data.time),
             "20%02d-%02d-%02d %02d:%02d:%02d", g_rtc_date.Year, g_rtc_date.Month, g_rtc_date.Date,
             g_rtc_time.Hours, g_rtc_time.Minutes, g_rtc_time.Seconds);
    g_newest_user_hr_data.hr = g_heart_rate;
    g_newest_user_hr_data.spo2 = g_spo2;
}

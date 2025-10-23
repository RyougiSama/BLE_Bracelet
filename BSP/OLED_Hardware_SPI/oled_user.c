#include "oled_user.h"

#include <stdio.h>

#include "atgm336h.h"
#include "max30102_user.h"
#include "mpu6050.h"
#include "oled_hardware_spi.h"
#include "step_count.h"
#include "task_scheduler.h"
#include "user_data.h"

// OLED显示字符串长度限制
#define OLED_MAX_STR_LEN 16   // 最大显示字符数
#define OLED_STR_BUF_SIZE 17  // 缓冲区大小 (包含'\0')

OLED_MainInterface g_curr_main_interface = OLED_STANDBY;
RTC_DateTypeDef g_rtc_date;
RTC_TimeTypeDef g_rtc_time;

static void OLED_STANDBY_Display(void) {
    OLED_ShowString(0, 0, (uint8_t*)"< BLE Bracelet >", 16);
    // 读取时间和日期
    read_bkup(&hrtc);
    HAL_RTC_GetDate(&hrtc, &g_rtc_date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &g_rtc_time, RTC_FORMAT_BIN);
    write_bkup(&hrtc);
    // 显示当前日期
    char date_str[16] = {0};
    snprintf(date_str, sizeof(date_str), "Date:%02d/%02d/20%02d", g_rtc_date.Date, g_rtc_date.Month,
             g_rtc_date.Year);
    OLED_ShowString(0, 2, (uint8_t*)date_str, 16);
    // 显示当前时间
    char time_str[16] = {0};
    snprintf(time_str, sizeof(time_str), "Time:%02d:%02d:%02d", g_rtc_time.Hours,
             g_rtc_time.Minutes, g_rtc_time.Seconds);
    OLED_ShowString(0, 4, (uint8_t*)time_str, 16);
    // 显示当前温度
    char temp_str[16] = {0};
    MPU6050_Read_All();
    snprintf(temp_str, sizeof(temp_str), "Temp:%.2f C", g_temp);
    OLED_ShowString(0, 6, (uint8_t*)temp_str, 16);
}

static void OLED_MAX30102_Display(void) {
    OLED_ShowString(0, 0, (uint8_t*)" MAX30102 Data", 16);
    // 测试MAX30102
    char blood_str[20] = {0};
    if (MAX30102_IsVaid()) {
        snprintf(blood_str, sizeof(blood_str), "HR:%3d SpO2:%3d", g_heart_rate, g_spo2);
        UserData_UpdateHealth();
    } else {
        snprintf(blood_str, sizeof(blood_str), "HR:--- SpO2:---");
    }
    OLED_ShowString(0, 2, (uint8_t*)blood_str, 16);
}

static void OLED_ClearNlines(uint8_t start_line, uint8_t num_lines) {
    for (uint8_t i = 0; i < num_lines; i++) {
        OLED_ShowString(0, start_line + i, (uint8_t*)"                ", 8);
    }
}

static void OLED_StepGPS_Display(void) {
    OLED_ShowString(0, 0, (uint8_t*)"Steps & GPS Data", 16);
    // 显示当前步数
    char step_str[20] = {0};
    snprintf(step_str, sizeof(step_str), "Steps:%5d", g_step);
    OLED_ShowString(0, 2, (uint8_t*)step_str, 16);
    // 显示GPS信息 (限制字符串长度为OLED_MAX_STR_LEN)
    char gps_utc_str[OLED_STR_BUF_SIZE] = {0};
    char gps_pos_str[OLED_STR_BUF_SIZE] = {0};
    if (Save_Data.isUsefull) {
        OLED_ClearNlines(6, 2);
        // 精确控制显示格式，确保不超过16个字符
        snprintf(gps_utc_str, OLED_STR_BUF_SIZE, "UTC:%.11s ",
                 Save_Data.UTCTime);  // "UTC:" + 11字符 = 15字符
        OLED_ShowString(0, 4, (uint8_t*)gps_utc_str, 16);
        snprintf(gps_pos_str, OLED_STR_BUF_SIZE, "Lat:%.10s%c", Save_Data.latitude,
                 Save_Data.N_S[0]);  // "Lat:" + 10字符 + 1方向 = 15字符
        OLED_ShowString(0, 6, (uint8_t*)gps_pos_str, 8);
        snprintf(gps_pos_str, OLED_STR_BUF_SIZE, "Lon:%.10s%c", Save_Data.longitude,
                 Save_Data.E_W[0]);  // "Lon:" + 10字符 + 1方向 = 15字符
        OLED_ShowString(0, 7, (uint8_t*)gps_pos_str, 8);
    } else {
        snprintf(gps_utc_str, OLED_STR_BUF_SIZE, "GPS Parse Error!");  // 正好16个字符
        OLED_ShowString(0, 4, (uint8_t*)gps_utc_str, 16);
        snprintf(gps_pos_str, OLED_STR_BUF_SIZE, "Outside Please! ");  // 正好16个字符
        OLED_ShowString(0, 6, (uint8_t*)gps_pos_str, 16);
    }
}

static void OLED_TEST_Display(void) {
    OLED_ShowString(0, 0, (uint8_t*)"OLED TEST MODE", 16);
    // 测试MPU6050
    MPU6050_Read_All();
    char accel_str[20] = {0};
    snprintf(accel_str, sizeof(accel_str), "A:%.2f %.2f %.2f", g_ax, g_ay, g_az);
    OLED_ShowString(0, 2, (uint8_t*)accel_str, 8);
    char gyro_str[20] = {0};
    snprintf(gyro_str, sizeof(gyro_str), "G:%.2f %.2f %.2f", g_gx, g_gy, g_gz);
    OLED_ShowString(0, 3, (uint8_t*)gyro_str, 8);
    char step_str[20] = {0};
    snprintf(step_str, sizeof(step_str), "Step:%d", g_step);
    OLED_ShowString(0, 4, (uint8_t*)step_str, 8);
    // 测试MAX30102
    char blood_str[20] = {0};
    if (MAX30102_IsVaid()) {
        snprintf(blood_str, sizeof(blood_str), "HR:%3d SpO2:%3d", g_heart_rate, g_spo2);
    } else {
        snprintf(blood_str, sizeof(blood_str), "HR:--- SpO2:---");
    }
    OLED_ShowString(0, 5, (uint8_t*)blood_str, 8);
}

void Task_OLED_Update(void) {
    switch (g_curr_main_interface) {
        case OLED_STANDBY:
            OLED_STANDBY_Display();
            break;
        case OLED_MAX30102:
            OLED_MAX30102_Display();
            break;
        case OLED_STEP_GPS:
            OLED_StepGPS_Display();
            break;
        case OLED_TEST:
            OLED_TEST_Display();
            break;
        default:
            break;
    }
}

void OLED_MoveToNextInterface(void) {
    g_curr_main_interface =
        (OLED_MainInterface)(((uint8_t)g_curr_main_interface + 1) % OLED_MAIN_INTERFACE_COUNT);
    OLED_Clear();
    if (g_curr_main_interface == OLED_MAX30102) {
        TaskScheduler_ResumeTask("Blood_Measure_Task");
    } else {
        TaskScheduler_SuspendTask("Blood_Measure_Task");
    }
}

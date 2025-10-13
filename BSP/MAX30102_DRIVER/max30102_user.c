#include "max30102_user.h"

#include <stdio.h>

#include "algorithm.h"
#include "oled_hardware_spi.h"

#define BUFFER_LENTH 500

uint32_t ir_buffer[BUFFER_LENTH];   // IR LED sensor data   红外数据，用于计算血氧
uint32_t red_buffer[BUFFER_LENTH];  // Red LED sensor data  红光数据，用于计算心率曲线以及计算心率
int32_t g_sp02;                     // SPO2 value
int8_t g_spo2_valid;                // indicator to show if the SP02 calculation is valid
int32_t g_heart_rate;               // heart rate value
int8_t g_hr_valid;                  // indicator to show if the heart rate calculation is valid

void max30102_test(void)
{
    uint32_t un_min, un_max;
    int i;
    uint8_t temp[6];

    un_min = 0x3FFFF;
    un_max = 0;

    // 读取前500个样本，并确定信号范围
    for (i = 0; i < BUFFER_LENTH; i++) {
        while (HAL_GPIO_ReadPin(MAX30102_INT_GPIO_Port, MAX30102_INT_Pin) == SET)  // 等待中断引脚
            ;

        max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);  // 读取传感器数据，赋值到temp中
        red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 |
                        (long)temp[2];  // 将值合并得到实际数字
        ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 |
                       (long)temp[5];  // 将值合并得到实际数字

        if (un_min > red_buffer[i])
            un_min = red_buffer[i];  // 更新信号最小值
        if (un_max < red_buffer[i])
            un_max = red_buffer[i];  // 更新信号最大值
    }

    // 计算前500个样本后的心率和SpO2（样本的前5秒）
    maxim_heart_rate_and_oxygen_saturation(ir_buffer, BUFFER_LENTH, red_buffer, &g_sp02,
                                           &g_spo2_valid, &g_heart_rate, &g_hr_valid);

    while (1) {
        // 读取和计算max30102数据，总体用缓存的500组数据分析，实际每读取100组新数据分析一次
        un_min = 0x3FFFF;
        un_max = 0;
        // 将前100组样本转储到内存中（实际没有），并将后400组样本移到顶部，将100-500缓存数据移位到0-400
        for (i = 100; i < 500; i++) {
            red_buffer[i - 100] = red_buffer[i];  // 将100-500缓存数据移位到0-400
            ir_buffer[i - 100] = ir_buffer[i];    // 将100-500缓存数据移位到0-400
            // 更新信号的最小值和最大值
            if (un_min > red_buffer[i])  // 寻找移位后0-400中的最小值
                un_min = red_buffer[i];
            if (un_max < red_buffer[i])  // 寻找移位后0-400中的最大值
                un_max = red_buffer[i];
        }
        // 在计算心率前取100组样本，取的数据放在400-500缓存数组中
        for (i = 400; i < 500; i++) {
            while (HAL_GPIO_ReadPin(MAX30102_INT_GPIO_Port, MAX30102_INT_Pin) ==
                   SET)  // 等待中断引脚
                ;

            max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);  // 读取传感器数据，赋值到temp中
            red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 |
                            (long)temp[2];  // 将值合并得到实际数字，数组400-500为新读取数据
            ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 |
                           (long)temp[5];  // 将值合并得到实际数字，数组400-500为新读取数据

            // 数据读取完成，继续处理
        }
        maxim_heart_rate_and_oxygen_saturation(
            ir_buffer, BUFFER_LENTH, red_buffer, &g_sp02, &g_spo2_valid, &g_heart_rate,
            &g_hr_valid);  // 传入500个心率和血氧数据计算传感器检测结论，反馈心率和血氧测试结果

        if ((1 == g_hr_valid) && (1 == g_spo2_valid) && (g_heart_rate < 120) && (g_sp02 < 101)) {
            // printf("HeartRate=%i, BloodOxyg=%i\r\n", g_heart_rate, g_sp02);
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "HR=%3d, SpO2=%3d", g_heart_rate, g_sp02);
            OLED_ShowString(0, 0, (uint8_t *)buffer, 16);
        }
    }
}
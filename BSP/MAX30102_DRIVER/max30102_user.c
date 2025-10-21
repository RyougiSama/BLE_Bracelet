#include "max30102_user.h"

#include <stdio.h>

#include "algorithm.h"
#include "max30102.h"
#include "oled_hardware_spi.h"

#define BUFFER_LENTH 500

uint32_t ir_buffer[BUFFER_LENTH];   // IR LED sensor data   红外数据，用于计算血氧
uint32_t red_buffer[BUFFER_LENTH];  // Red LED sensor data  红光数据，用于计算心率曲线以及计算心率
int32_t g_spo2;                     // SPO2 value
int8_t g_spo2_valid;                // indicator to show if the SP02 calculation is valid
int32_t g_heart_rate;               // heart rate value
int8_t g_hr_valid;                  // indicator to show if the heart rate calculation is valid

/**
 * @brief MAX30102系统初始化
 *
 */
void MAX30102_System_Init(void) {
    max30102_init();  // max30102初始化
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
    maxim_heart_rate_and_oxygen_saturation(ir_buffer, BUFFER_LENTH, red_buffer, &g_spo2,
                                           &g_spo2_valid, &g_heart_rate, &g_hr_valid);
}

#if 0
void Task_BloodMeasure(void) {
    uint32_t un_min, un_max;
    int i;
    uint8_t temp[6];

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
        while (HAL_GPIO_ReadPin(MAX30102_INT_GPIO_Port, MAX30102_INT_Pin) == SET)  // 等待中断引脚
            ;

        max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);  // 读取传感器数据，赋值到temp中
        red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 |
                        (long)temp[2];  // 将值合并得到实际数字，数组400-500为新读取数据
        ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 |
                       (long)temp[5];  // 将值合并得到实际数字，数组400-500为新读取数据

        // 数据读取完成，继续处理
    }
    maxim_heart_rate_and_oxygen_saturation(
        ir_buffer, BUFFER_LENTH, red_buffer, &g_spo2, &g_spo2_valid, &g_heart_rate,
        &g_hr_valid);  // 传入500个心率和血氧数据计算传感器检测结论，反馈心率和血氧测试结果
}
#endif

// 可调参数：每次函数最多读取多少样本（根据实时性/开销调整）
#ifndef SAMPLE_BATCH
#define SAMPLE_BATCH 10
#endif

void Task_BloodMeasure(void) {
    uint8_t temp[6];
    uint16_t i;
    // 静态持久化变量（保留在函数间）
    static uint16_t write_index = 0;  // 下一个写入位置（0..BUFFER_LENTH-1）
    static uint16_t filled = 0;       // 已填充的样本数量（<= BUFFER_LENTH）
    static uint16_t new_count = 0;    // 自上次分析以来新增样本数

    // 临时线性化数组（静态以节省栈）
    static uint32_t tmp_ir[BUFFER_LENTH];
    static uint32_t tmp_red[BUFFER_LENTH];

    // 每次最多读取 SAMPLE_BATCH 条数据；若没有数据则立即返回（非阻塞）
    for (i = 0; i < SAMPLE_BATCH; i++) {
        while (HAL_GPIO_ReadPin(MAX30102_INT_GPIO_Port, MAX30102_INT_Pin) == SET)
            ;
        // 读取一帧 FIFO 数据（6 字节）
        max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

        // 合并为 18-bit 数据（和你原来合并方式一致）
        uint32_t red_val =
            ((uint32_t)(temp[0] & 0x03) << 16) | ((uint32_t)temp[1] << 8) | (uint32_t)temp[2];
        uint32_t ir_val =
            ((uint32_t)(temp[3] & 0x03) << 16) | ((uint32_t)temp[4] << 8) | (uint32_t)temp[5];

        // 写入环形缓冲区
        red_buffer[write_index] = red_val;
        ir_buffer[write_index] = ir_val;

        // 更新索引与计数
        write_index++;
        if (write_index >= BUFFER_LENTH)
            write_index = 0;

        if (filled < BUFFER_LENTH)
            filled++;

        new_count++;
    }

    // 仅在缓冲区已满并且累计新样本 >= 100 时才做一次完整分析
    if ((filled >= BUFFER_LENTH) && (new_count >= 100)) {
        // 将环形缓冲线性化为 tmp_*：按时间顺序 oldest -> newest
        // oldest 索引就是 write_index（因为 write_index 指向下一个将被覆盖的位置）
        for (uint16_t k = 0; k < BUFFER_LENTH; k++) {
            uint16_t idx = write_index + k;
            if (idx >= BUFFER_LENTH)
                idx -= BUFFER_LENTH;
            tmp_ir[k] = ir_buffer[idx];
            tmp_red[k] = red_buffer[idx];
        }

        // 调用分析函数（使用线性化数组）
        maxim_heart_rate_and_oxygen_saturation(tmp_ir, BUFFER_LENTH, tmp_red, &g_spo2,
                                               &g_spo2_valid, &g_heart_rate, &g_hr_valid);

        // 重置新增样本计数（等待下一个 100 个新样本）
        new_count = 0;
    }
}

bool MAX30102_IsVaid(void) {
    if ((1 == g_hr_valid) && (1 == g_spo2_valid) && (g_heart_rate < 120) && (g_spo2 < 101)) {
        // printf("HeartRate=%i, BloodOxyg=%i\r\n", g_heart_rate, g_spo2);
        // char buffer[20];
        // snprintf(buffer, sizeof(buffer), "HR=%3d, SpO2=%3d", g_heart_rate, g_spo2);
        // OLED_ShowString(0, 0, (uint8_t *)buffer, 16);
        return true;
    }
    return false;
}

void max30102_test(void) {
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
    maxim_heart_rate_and_oxygen_saturation(ir_buffer, BUFFER_LENTH, red_buffer, &g_spo2,
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
            ir_buffer, BUFFER_LENTH, red_buffer, &g_spo2, &g_spo2_valid, &g_heart_rate,
            &g_hr_valid);  // 传入500个心率和血氧数据计算传感器检测结论，反馈心率和血氧测试结果

        if ((1 == g_hr_valid) && (1 == g_spo2_valid) && (g_heart_rate < 120) && (g_spo2 < 101)) {
            // printf("HeartRate=%i, BloodOxyg=%i\r\n", g_heart_rate, g_spo2);
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "HR=%3d, SpO2=%3d", g_heart_rate, g_spo2);
            OLED_ShowString(0, 0, (uint8_t *)buffer, 16);
        }
    }
}

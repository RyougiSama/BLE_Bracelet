#include "user_init.h"

#include <stdio.h>
#include "app_tasks.h"
#include "usart.h"
#include "uart_user.h"
#include "oled_hardware_spi.h"
#include "mpu6050.h"
#include "blood_measure_task.h"

/**
 * @brief 用户自定义初始化函数
 *
 */
void User_Init(void)
{
    // BLE Uart 空闲中断接收使能并关闭 DMA 过半中断
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_uart_command_buffer, UART_USER_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    // OLED 初始化
    OLED_Init();
    OLED_Clear();
    // MPU6050 初始化
    if (MPU6050_Init() != HAL_OK) {
        OLED_ShowString(0, 6, (uint8_t *)"MPU6050 ERR!", 16);
    }
    // MAX30102 初始化
    BloodMeasure_Init();
    // 初始化应用任务
    AppTasks_Init();
}

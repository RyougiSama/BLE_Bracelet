#include "user_init.h"

#include <stdio.h>

#include "app_tasks.h"
#include "max30102_user.h"
#include "mpu6050.h"
#include "oled_hardware_spi.h"
#include "tim.h"
#include "uart_user.h"
#include "usart.h"

/**
 * @brief 用户自定义初始化函数
 *
 */
void User_Init(void) {
    // BLE Uart 空闲中断接收使能并关闭 DMA 过半中断
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_uart_command_buffer, UART_USER_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    // OLED 初始化
    OLED_Init();
    OLED_Clear();
    // MPU6050 初始化
    if (MPU6050_Init() != HAL_OK) {
        OLED_ShowString(0, 0, (uint8_t *)"MPU6050 ERR!", 16);
        while (true) {
        }
    }
    // MAX30102 初始化
    MAX30102_System_Init();
    // 启动计步定时器6，50ms中断一次
    // 清除定时器初始化过程中的更新中断标志，避免定时器一启动就中断
    __HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);
    // 使能定时器6更新中断并启动定时器
    HAL_TIM_Base_Start_IT(&htim6);
    // 初始化应用任务
    AppTasks_Init();
}

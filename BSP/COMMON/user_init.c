#include "user_init.h"

#include <stdio.h>
#include "app_tasks.h"
#include "usart.h"
#include "uart_user.h"

#include "oled_hardware_spi.h"

/**
 * @brief 用户自定义初始化函数
 *
 */
void User_Init(void)
{
    // Uart 空闲中断接收使能并关闭 DMA 过半中断
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_uart_command_buffer, UART_USER_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    // 初始化应用任务
    AppTasks_Init();

    // printf("Hello world!");
    // OLED 初始化
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t *)"Hello world!", 16);
    OLED_ShowNum(0, 2, 1234567890, 10, 16);
}

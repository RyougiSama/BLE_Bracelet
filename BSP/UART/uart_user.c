#include "uart_user.h"

#include <stdio.h>

#include "command.h"
#include "task_scheduler.h"
#include "usart.h"
#include "user_init.h"

uint8_t g_uart_command_buffer[UART_USER_BUFFER_SIZE];  // UART command buffer

// 中值滤波相关变量
#define FILTER_BUFFER_SIZE 3                           // 滤波缓冲区大小

// 滤波控制变量
static bool filter_enabled = false;  // 滤波使能标志

/**
 * @brief 获取中值（冒泡排序）
 *
 * @param values 待排序的数组
 * @param size 数组大小
 * @return uint16_t 中值
 */
static uint16_t GetMedian(uint16_t values[], uint8_t size)
{
    uint16_t temp;
    uint8_t i, j;

    // 冒泡排序
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (values[j] > values[j + 1]) {
                temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }

    // 返回中值
    return values[size / 2];
}

/**
 * @brief 处理 UART 接收到的数据，该函数应在任务调度器中调用
 *
 */
void Uart_DataProcess(void)
{
    uint8_t command_length = Command_GetCommand(g_uart_command_buffer);

    // 收到正确格式数据包时的解析
    if (command_length) {

        // 根据滤波使能标志决定是否应用中值滤波
        if (filter_enabled) {
            // 应用中值滤波
        } else {
            // 直接使用原始数据，不进行滤波
        }
    }
}

/**
 * @brief 启用或禁用中值滤波
 * @param enable true启用滤波，false禁用滤波
 */
void Uart_SetFilterEnabled(bool enable)
{
    filter_enabled = enable;

    // 如果重新启用滤波，清空滤波缓冲区以避免使用旧数据
    if (enable) {
    }
}

// 中断空闲接收回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // Check if the UART instance is USART2
    if (huart->Instance == USART2) {
        Command_Write(g_uart_command_buffer, Size);
        // Re-enable the reception event
        HAL_UARTEx_ReceiveToIdle_DMA(huart, g_uart_command_buffer, UART_USER_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    }
}

/**
 * @brief 重定向c库函数printf到DEBUG_USARTx
 *
 * @param ch
 * @param f
 * @return int
 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xffff);
    return ch;
}

/**
 * @brief: 重定向c库函数getchar,scanf到DEBUG_USARTx
 * @param f
 * @return int
 *
 */
int fgetc(FILE *f)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&huart2, &ch, 1, 0xffff);
    return ch;
}

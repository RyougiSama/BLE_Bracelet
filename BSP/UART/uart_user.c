#include "uart_user.h"

#include <stdio.h>

#include "command.h"
#include "mpu6050.h"
#include "task_scheduler.h"
#include "usart.h"
#include "user_data.h"
#include "user_init.h"
#include "step_count.h"
#include "atgm336h.h"

typedef enum {
    COMMAND_TEMPERATURE = 0x01,
    COMMAND_HEALTH = 0x02,
    COMMAND_STEP_COUNT = 0x03,
    COMMAND_GPS = 0x04,
} CommandCodeType;

uint8_t g_uart_command_buffer[UART_USER_BUFFER_SIZE];  // UART command buffer

static void CommandCode_Temperature(void) {
    MPU6050_Read_All();
    printf("Temperature: %.2f C\n", g_temp);
}

static void CommandCode_Health(void) {
    if (g_newest_user_hr_data.hr == 0 && g_newest_user_hr_data.spo2 == 0) {
        printf("No valid health data available.\n");
        return;
    }
    printf("Last Valid Health Data:\n");
    printf("Detection Time: %s\n", g_newest_user_hr_data.time);
    printf("Heart Rate: %d bpm, SpO2: %d %%\n", g_newest_user_hr_data.hr,
           g_newest_user_hr_data.spo2);
}

static void CommandCode_StepCount(void) {
    printf("Current Step Count: %d steps\n", g_step);
}

static void CommandCode_GPS(void) {
    if (Save_Data.isUsefull) {
        printf("GPS Data:\n");
        printf("UTC Time: %s\n", Save_Data.UTCTime);
        printf("Latitude: %s %s\n", Save_Data.latitude, Save_Data.N_S);
        printf("Longitude: %s %s\n", Save_Data.longitude, Save_Data.E_W);
    } else {
        printf("No valid GPS data available.\n");
    }
}

static void CommandCode_Handle(CommandCodeType cmd_code) {
    // printf("Processing Command Code: 0x%02X\n", cmd_code);
    switch (cmd_code) {
        case COMMAND_TEMPERATURE:
            CommandCode_Temperature();
            break;
        case COMMAND_HEALTH:
            CommandCode_Health();
            break;
        case COMMAND_STEP_COUNT:
            CommandCode_StepCount();
            break;
        case COMMAND_GPS:
            CommandCode_GPS();
            break;
        default:
            break;
    }
}

/**
 * @brief 处理 UART 接收到的数据，该函数应在任务调度器中调用
 *
 */
void Task_BLE_DataReceiveProc(void) {
    uint8_t command_length = Command_GetCommand(g_uart_command_buffer);

    // 收到正确格式数据包时的解析
    if (command_length) {
        /*  printf("Received Command: ");
         for (uint8_t i = 0; i < command_length; i++) {
             printf("0x%02X ", g_uart_command_buffer[i]);
         } */
        CommandCode_Handle((CommandCodeType)g_uart_command_buffer[2]);
    }
}

// 中断空闲接收回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
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
int fputc(int ch, FILE* f) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xffff);
    return ch;
}

/**
 * @brief: 重定向c库函数getchar,scanf到DEBUG_USARTx
 * @param f
 * @return int
 *
 */
int fgetc(FILE* f) {
    uint8_t ch = 0;
    HAL_UART_Receive(&huart2, &ch, 1, 0xffff);
    return ch;
}

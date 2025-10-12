/**
 * @file    max30102_example.h
 * @brief   MAX30102 example header file for interrupt-driven operation
 * @author  Generated for STM32 HAL
 * @date    2024
 */

#ifndef MAX30102_EXAMPLE_H
#define MAX30102_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>

#include "max30102_algorithm.h"
#include "max30102_hal.h"

/* Exported constants --------------------------------------------------------*/
#define DATA_BUFFER_SIZE 500    // 5 seconds at 100 Hz
#define SAMPLES_PER_UPDATE 100  // 1 second update rate

/* Exported variables --------------------------------------------------------*/
extern volatile bool data_ready_flag;

/* Exported function prototypes ----------------------------------------------*/

/**
 * @brief Initialize MAX30102 system
 */
bool MAX30102_System_Init(void);

/**
 * @brief MAX30102 task function for scheduled execution
 */
void MAX30102_Task_Handler(void);

/**
 * @brief Get current heart rate and SpO2 results
 */
bool MAX30102_Get_Results(int32_t *hr_result, int32_t *spo2_result, bool *hr_valid,
                          bool *spo2_valid);

/**
 * @brief Check if MAX30102 has new data available
 */
bool MAX30102_Has_New_Data(void);

/**
 * @brief MAX30102 data ready interrupt handler
 */
void MAX30102_DataReady_ISR(void);

/**
 * @brief Legacy main function (for backward compatibility)
 */
void MAX30102_Example_Main(void);

#ifdef __cplusplus
}
#endif

#endif /* MAX30102_EXAMPLE_H */
/**
 * @file max30102_example.c
 * @brief MAX30102 task-based implementation for STM32 HAL
 */

/* Includes ------------------------------------------------------------------*/
#include "max30102_example.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "max30102_algorithm.h"
#include "max30102_config.h"
#include "max30102_hal.h"

/* Private defines -----------------------------------------------------------*/
#define DATA_BUFFER_SIZE 500     // 5 seconds of data at 100Hz
#define SAMPLES_PER_UPDATE 100   // Update every 100 samples (1 second)
#define UART_TIMEOUT 1000        // UART timeout for debug output
#define TASK_PRINT_INTERVAL 100  // Print interval in ms for task mode

/* Task states ---------------------------------------------------------------*/
typedef enum {
    MAX30102_STATE_UNINITIALIZED = 0,
    MAX30102_STATE_INITIALIZING,
    MAX30102_STATE_COLLECTING_INITIAL_DATA,
    MAX30102_STATE_MONITORING,
    MAX30102_STATE_ERROR
} max30102_state_t;

/* Private variables ---------------------------------------------------------*/
static uint32_t red_buffer[DATA_BUFFER_SIZE];
static uint32_t ir_buffer[DATA_BUFFER_SIZE];
static algorithm_results_t algorithm_results;

// Task scheduling variables
static max30102_state_t system_state = MAX30102_STATE_UNINITIALIZED;
volatile bool data_ready_flag = false;
static volatile uint32_t sample_count = 0;
static volatile bool initial_data_collected = false;
static volatile bool calculation_needed = false;
static volatile bool new_results_available = false;
static uint32_t buffer_index = 0;
static uint32_t last_print_time = 0;
static uint32_t min_value = 0x3FFFF;
static uint32_t max_value = 0;

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
static bool MAX30102_Hardware_Init(void);
static bool MAX30102_Interrupt_Init(void);
static void MAX30102_PrintResults(uint32_t red_val, uint32_t ir_val);
static void MAX30102_ShiftBuffer(void);
static void MAX30102_Task_CollectInitialData(void);
static void MAX30102_Task_ProcessMonitoring(void);
static void MAX30102_CheckSensorStatus(void);

/* Public API Functions ------------------------------------------------------*/

/**
 * @brief Initialize MAX30102 system
 * @retval true if initialization successful, false otherwise
 */
bool MAX30102_System_Init(void)
{
    // Initialize hardware
    if (!MAX30102_Hardware_Init()) {
        system_state = MAX30102_STATE_ERROR;
        return false;
    }

    // Initialize interrupt (Note: GPIO should be configured in CubeMX)
    if (!MAX30102_Interrupt_Init()) {
        system_state = MAX30102_STATE_ERROR;
        return false;
    }

    // Initialize variables
    sample_count = 0;
    buffer_index = 0;
    initial_data_collected = false;
    calculation_needed = false;
    new_results_available = false;
    data_ready_flag = false;
    min_value = 0x3FFFF;
    max_value = 0;
    last_print_time = 0;

    // Clear algorithm results
    memset(&algorithm_results, 0, sizeof(algorithm_results));

    system_state = MAX30102_STATE_COLLECTING_INITIAL_DATA;

    return true;
}

/**
 * @brief MAX30102 task function for scheduled execution
 */
void MAX30102_Task_Handler(void)
{
#if 0
    // Check for new data by polling if interrupt is not working
    static uint32_t last_poll_time = 0;
    uint32_t current_time = HAL_GetTick();

    // Poll every 10ms if no interrupt received
    if (current_time - last_poll_time >= 10) {
        last_poll_time = current_time;

        // Check FIFO status manually
        uint8_t write_ptr, read_ptr;
        if (MAX30102_ReadReg(0x04, &write_ptr) &&
            MAX30102_ReadReg(0x06, &read_ptr)) {  // FIFO_WR_PTR and FIFO_RD_PTR
            if (write_ptr != read_ptr) {
                // New data available, simulate interrupt
                data_ready_flag = true;
            }
        }
    }
#endif
    switch (system_state) {
        case MAX30102_STATE_UNINITIALIZED:
            // System not initialized, do nothing
            break;

        case MAX30102_STATE_COLLECTING_INITIAL_DATA:
            MAX30102_Task_CollectInitialData();
            // MAX30102_CheckSensorStatus();
            break;

        case MAX30102_STATE_MONITORING:
            MAX30102_Task_ProcessMonitoring();
            // MAX30102_CheckSensorStatus();
            break;

        case MAX30102_STATE_ERROR:
            // Error state, could implement recovery logic here
            break;

        default:
            system_state = MAX30102_STATE_ERROR;
            break;
    }
}

/**
 * @brief Get current heart rate and SpO2 results
 */
bool MAX30102_Get_Results(int32_t *hr_result, int32_t *spo2_result, bool *hr_valid,
                          bool *spo2_valid)
{
    if (hr_result)
        *hr_result = algorithm_results.heart_rate;
    if (spo2_result)
        *spo2_result = algorithm_results.spo2;
    if (hr_valid)
        *hr_valid = algorithm_results.hr_valid;
    if (spo2_valid)
        *spo2_valid = algorithm_results.spo2_valid;

    if (new_results_available) {
        new_results_available = false;
        return true;
    }

    return false;
}

/**
 * @brief Check if MAX30102 has new data available
 */
bool MAX30102_Has_New_Data(void)
{
    return data_ready_flag;
}

/* Legacy Functions ----------------------------------------------------------*/

/**
 * @brief Legacy main function (for backward compatibility)
 */
void MAX30102_Example_Main(void)
{
    // Initialize system
    if (!MAX30102_System_Init()) {
        while (1) {
            HAL_Delay(1000);
        }
    }

    // Legacy main loop
    while (1) {
        MAX30102_Task_Handler();
        HAL_Delay(20);  // 50Hz task execution
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Collect initial data for algorithm
 */
static void MAX30102_Task_CollectInitialData(void)
{
    // Check if we have new data from interrupt
    if (data_ready_flag) {
        data_ready_flag = false;

        uint32_t red_value, ir_value;

        // Read FIFO data
        if (MAX30102_ReadFIFO(&red_value, &ir_value)) {
            // Store data in buffer
            if (buffer_index < DATA_BUFFER_SIZE) {
                red_buffer[buffer_index] = red_value;
                ir_buffer[buffer_index] = ir_value;
                buffer_index++;
                sample_count++;

                // Update signal range
                if (red_value < min_value)
                    min_value = red_value;
                if (red_value > max_value)
                    max_value = red_value;
            }

            // Check if initial data collection is complete
            if (buffer_index >= DATA_BUFFER_SIZE) {
                // Calculate initial algorithm results
                Algorithm_CalculateHeartRateAndSpO2(ir_buffer, DATA_BUFFER_SIZE, red_buffer,
                                                    &algorithm_results);

                initial_data_collected = true;
                system_state = MAX30102_STATE_MONITORING;
                buffer_index = 0;  // Reset for monitoring mode
            }
        }
    }
}

/**
 * @brief Process monitoring data
 */
static void MAX30102_Task_ProcessMonitoring(void)
{
    uint32_t current_time = HAL_GetTick();

    // Process new data from interrupt
    if (data_ready_flag) {
        data_ready_flag = false;

        uint32_t red_value, ir_value;

        // Read FIFO data
        if (MAX30102_ReadFIFO(&red_value, &ir_value)) {
            // Shift buffer if necessary (sliding window)
            if (buffer_index >= SAMPLES_PER_UPDATE) {
                MAX30102_ShiftBuffer();
                buffer_index = DATA_BUFFER_SIZE - SAMPLES_PER_UPDATE;
            }

            // Store new data
            red_buffer[buffer_index] = red_value;
            ir_buffer[buffer_index] = ir_value;
            buffer_index++;
            sample_count++;

            // Update signal range
            if (red_value < min_value)
                min_value = red_value;
            if (red_value > max_value)
                max_value = red_value;

            // Print real-time data (throttled)
            if (current_time - last_print_time >= TASK_PRINT_INTERVAL) {
                MAX30102_PrintResults(red_value, ir_value);
                last_print_time = current_time;
            }

            // Check if we need to run algorithm (every 100 samples)
            if (sample_count % SAMPLES_PER_UPDATE == 0) {
                calculation_needed = true;
            }
        }
    }

    // Process algorithm calculation when needed
    if (calculation_needed && buffer_index >= SAMPLES_PER_UPDATE) {
        calculation_needed = false;

        // Calculate heart rate and SpO2
        Algorithm_CalculateHeartRateAndSpO2(ir_buffer, DATA_BUFFER_SIZE, red_buffer,
                                            &algorithm_results);

        new_results_available = true;
    }
}

/**
 * @brief Initialize MAX30102 hardware
 */
static bool MAX30102_Hardware_Init(void)
{
    uint8_t part_id, rev_id;

    // Reset and initialize sensor
    if (!MAX30102_Reset()) {
        return false;
    }

    HAL_Delay(100);  // Wait for reset

    if (!MAX30102_Init()) {
        return false;
    }

    // Verify device ID
    if (MAX30102_GetDeviceID(&part_id, &rev_id)) {
        if (part_id != 0x15) {  // Expected part ID for MAX30102
            return false;
        }
    } else {
        return false;
    }
    return true;
}

/**
 * @brief Initialize MAX30102 interrupt
 */
static bool MAX30102_Interrupt_Init(void)
{
    // Enable EXTI4 interrupt
    HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    return true;
}

/**
 * @brief Shift data buffer (sliding window)
 */
static void MAX30102_ShiftBuffer(void)
{
    uint32_t i;

    // Shift data: move samples [100..499] to [0..399]
    for (i = SAMPLES_PER_UPDATE; i < DATA_BUFFER_SIZE; i++) {
        red_buffer[i - SAMPLES_PER_UPDATE] = red_buffer[i];
        ir_buffer[i - SAMPLES_PER_UPDATE] = ir_buffer[i];
    }
}

/**
 * @brief Print measurement results
 */
static void MAX30102_PrintResults(uint32_t red_val, uint32_t ir_val)
{
    (void)red_val;
    (void)ir_val;
}

/**
 * @brief Check MAX30102 sensor status for debugging
 */
static void MAX30102_CheckSensorStatus(void)
{
    static uint32_t last_check_time = 0;
    uint32_t current_time = HAL_GetTick();

    // Check status every 1 second
    if (current_time - last_check_time >= 1000) {
        last_check_time = current_time;

        uint8_t int_status1, int_status2;
        uint8_t fifo_wr_ptr, fifo_rd_ptr;

        // Read interrupt status
        if (MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_1, &int_status1) &&
            MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_2, &int_status2) &&
            MAX30102_ReadReg(0x04, &fifo_wr_ptr) &&  // FIFO_WR_PTR
            MAX30102_ReadReg(0x06, &fifo_rd_ptr)) {  // FIFO_RD_PTR

            // If we have data in FIFO but no interrupt flag, there's an interrupt issue
            if (fifo_wr_ptr != fifo_rd_ptr && !data_ready_flag) {
                // Force set data ready flag - this indicates interrupt hardware issue
                data_ready_flag = true;
            }
        }
    }
}

/* End of file ---------------------------------------------------------------*/
/**
 * @file max30102_config.h
 * @brief MAX30102 driver configuration file
 * @author Configuration template
 * @date 2025
 *
 * This file contains all configurable parameters for the MAX30102 driver.
 * Modify these settings according to your hardware configuration and requirements.
 */

#ifndef __MAX30102_CONFIG_H
#define __MAX30102_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Hardware Configuration ----------------------------------------------------*/

/**
 * @brief I2C Handle Configuration
 * Change this macro to match your I2C peripheral
 * Common options: hi2c1, hi2c2, hi2c3
 */
#define MAX30102_I2C_HANDLE hi2c1

/**
 * @brief I2C Timeout Configuration (milliseconds)
 * Adjust based on your I2C clock speed and system requirements
 */
#define MAX30102_I2C_TIMEOUT 1000

/**
 * @brief MAX30102 Interrupt Pin Configuration (Optional)
 * Define these if you want to use interrupt-driven data acquisition
 */
// #define MAX30102_INT_PORT           GPIOB
// #define MAX30102_INT_PIN            GPIO_PIN_4
// #define MAX30102_INT_EXTI_IRQn      EXTI4_IRQn

/* Sensor Configuration ------------------------------------------------------*/

/**
 * @brief Default LED Current Settings (0-255, ~0.2mA per step)
 * LED1 = Red LED, LED2 = IR LED
 * Higher values = more sensitive but more power consumption
 */
#define MAX30102_DEFAULT_LED1_CURRENT 0x24   // ~7.2mA
#define MAX30102_DEFAULT_LED2_CURRENT 0x24   // ~7.2mA
#define MAX30102_DEFAULT_PILOT_CURRENT 0x7F  // ~25.4mA

/**
 * @brief Default Sampling Configuration
 */
#define MAX30102_DEFAULT_SAMPLE_RATE MAX30102_SAMPLE_RATE_100HZ
#define MAX30102_DEFAULT_PULSE_WIDTH MAX30102_PULSE_WIDTH_411US
#define MAX30102_DEFAULT_ADC_RANGE MAX30102_ADC_RANGE_4096

/**
 * @brief Default Operating Mode
 * Options: MAX30102_MODE_HR_ONLY, MAX30102_MODE_SPO2, MAX30102_MODE_MULTI_LED
 */
#define MAX30102_DEFAULT_MODE MAX30102_MODE_SPO2

/**
 * @brief FIFO Configuration
 * FIFO_A_FULL: Trigger interrupt when FIFO has (32-value) samples
 * FIFO_ROLLOVER: Enable/disable FIFO rollover on full
 */
#define MAX30102_FIFO_A_FULL_VALUE 15    // 0-15, triggers when FIFO has (32-15)=17 samples
#define MAX30102_FIFO_ROLLOVER_ENABLE 0  // 0=disabled, 1=enabled

/* Algorithm Configuration ---------------------------------------------------*/

/**
 * @brief Data Buffer Size
 * Must be large enough to contain several heart beat cycles
 * Recommended: 5 seconds of data (500 samples at 100Hz)
 */
#define MAX30102_ALGORITHM_BUFFER_SIZE 500

/**
 * @brief Samples Per Calculation Update
 * How many new samples to collect before recalculating HR/SpO2
 * Recommended: 100 samples (1 second at 100Hz)
 */
#define MAX30102_SAMPLES_PER_UPDATE 100

/**
 * @brief Heart Rate Limits (BPM)
 * Results outside this range will be marked as invalid
 */
#define MAX30102_HR_MIN_BPM 40
#define MAX30102_HR_MAX_BPM 200

/**
 * @brief SpO2 Limits (%)
 * Results outside this range will be marked as invalid
 */
#define MAX30102_SPO2_MIN_PERCENT 70
#define MAX30102_SPO2_MAX_PERCENT 100

/* Debug and Output Configuration --------------------------------------------*/

/**
 * @brief Enable/Disable Debug Output
 * Set to 1 to enable printf debugging (requires UART configuration)
 */
#define MAX30102_DEBUG_ENABLE 1

/**
 * @brief UART Handle for Debug Output (if debug enabled)
 */
#if MAX30102_DEBUG_ENABLE
#define MAX30102_DEBUG_UART huart1
#define MAX30102_DEBUG_UART_TIMEOUT 1000
#endif

/**
 * @brief Enable/Disable Real-time Data Plotting
 * Set to 1 to output data in format suitable for serial plotter
 */
#define MAX30102_PLOTTER_OUTPUT_ENABLE 0

/* Advanced Configuration ----------------------------------------------------*/

/**
 * @brief Temperature Measurement Enable
 * Set to 1 to enable temperature reading from MAX30102
 */
#define MAX30102_TEMPERATURE_ENABLE 0

/**
 * @brief Proximity Detection Enable
 * Set to 1 to enable finger detection using proximity sensing
 */
#define MAX30102_PROXIMITY_ENABLE 0

/**
 * @brief Custom Filter Coefficients
 * Advanced users can modify these for different filter characteristics
 */
#define MAX30102_HAMMING_COEFF_0 41
#define MAX30102_HAMMING_COEFF_1 276
#define MAX30102_HAMMING_COEFF_2 512
#define MAX30102_HAMMING_COEFF_3 276
#define MAX30102_HAMMING_COEFF_4 41

/**
 * @brief Peak Detection Parameters
 * Fine-tune these for different signal characteristics
 */
#define MAX30102_PEAK_MIN_DISTANCE 8    // Minimum distance between peaks
#define MAX30102_PEAK_MAX_COUNT 5       // Maximum peaks to analyze
#define MAX30102_VALLEY_SEARCH_RANGE 5  // Range to search for precise valleys

/* Validation Macros ---------------------------------------------------------*/

// Compile-time validation of configuration
#if MAX30102_ALGORITHM_BUFFER_SIZE < 200
#error "MAX30102_ALGORITHM_BUFFER_SIZE must be at least 200"
#endif

#if MAX30102_SAMPLES_PER_UPDATE > MAX30102_ALGORITHM_BUFFER_SIZE / 2
#error "MAX30102_SAMPLES_PER_UPDATE must be less than half of ALGORITHM_BUFFER_SIZE"
#endif

#ifdef __cplusplus
}
#endif

#endif /* MAX30102_CONFIG_H */
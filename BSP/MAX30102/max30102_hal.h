/**
 * @file max30102_hal.h
 * @brief MAX30102 heart rate and oxygen saturation sensor driver for STM32 HAL
 * @author Converted from mbed version
 * @date 2025
 *
 * This driver provides functions for MAX30102 sensor communication and data acquisition
 * Compatible with STM32 HAL I2C library
 */

#ifndef __MAX30102_HAL_H
#define __MAX30102_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#include "i2c.h"

/* Hardware Configuration Macros --------------------------------------------*/
#ifndef MAX30102_I2C_HANDLE
#define MAX30102_I2C_HANDLE hi2c1  // Default to I2C1, can be overridden
#endif

#ifndef MAX30102_I2C_TIMEOUT
#define MAX30102_I2C_TIMEOUT 1000  // I2C timeout in milliseconds
#endif

/* I2C Address Definition ----------------------------------------------------*/
#define MAX30102_I2C_ADDR 0x57                                 // 7-bit address
#define MAX30102_I2C_WRITE_ADDR (MAX30102_I2C_ADDR << 1)       // 0xAE
#define MAX30102_I2C_READ_ADDR ((MAX30102_I2C_ADDR << 1) | 1)  // 0xAF

/* Register Addresses --------------------------------------------------------*/
#define MAX30102_REG_INTR_STATUS_1 0x00
#define MAX30102_REG_INTR_STATUS_2 0x01
#define MAX30102_REG_INTR_ENABLE_1 0x02
#define MAX30102_REG_INTR_ENABLE_2 0x03
#define MAX30102_REG_FIFO_WR_PTR 0x04
#define MAX30102_REG_OVF_COUNTER 0x05
#define MAX30102_REG_FIFO_RD_PTR 0x06
#define MAX30102_REG_FIFO_DATA 0x07
#define MAX30102_REG_FIFO_CONFIG 0x08
#define MAX30102_REG_MODE_CONFIG 0x09
#define MAX30102_REG_SPO2_CONFIG 0x0A
#define MAX30102_REG_LED1_PA 0x0C
#define MAX30102_REG_LED2_PA 0x0D
#define MAX30102_REG_PILOT_PA 0x10
#define MAX30102_REG_MULTI_LED_CTRL1 0x11
#define MAX30102_REG_MULTI_LED_CTRL2 0x12
#define MAX30102_REG_TEMP_INTR 0x1F
#define MAX30102_REG_TEMP_FRAC 0x20
#define MAX30102_REG_TEMP_CONFIG 0x21
#define MAX30102_REG_PROX_INT_THRESH 0x30
#define MAX30102_REG_REV_ID 0xFE
#define MAX30102_REG_PART_ID 0xFF

/* Configuration Constants ---------------------------------------------------*/
#define MAX30102_SAMPLE_RATE_50HZ 0x00
#define MAX30102_SAMPLE_RATE_100HZ 0x01
#define MAX30102_SAMPLE_RATE_200HZ 0x02
#define MAX30102_SAMPLE_RATE_400HZ 0x03
#define MAX30102_SAMPLE_RATE_800HZ 0x04
#define MAX30102_SAMPLE_RATE_1000HZ 0x05
#define MAX30102_SAMPLE_RATE_1600HZ 0x06
#define MAX30102_SAMPLE_RATE_3200HZ 0x07

#define MAX30102_PULSE_WIDTH_69US 0x00
#define MAX30102_PULSE_WIDTH_118US 0x01
#define MAX30102_PULSE_WIDTH_215US 0x02
#define MAX30102_PULSE_WIDTH_411US 0x03

#define MAX30102_ADC_RANGE_2048 0x00
#define MAX30102_ADC_RANGE_4096 0x01
#define MAX30102_ADC_RANGE_8192 0x02
#define MAX30102_ADC_RANGE_16384 0x03

#define MAX30102_MODE_HR_ONLY 0x02
#define MAX30102_MODE_SPO2 0x03
#define MAX30102_MODE_MULTI_LED 0x07

/* Data Buffer Size ----------------------------------------------------------*/
#define MAX30102_BUFFER_SIZE 500

/* External I2C Handle Declaration -------------------------------------------*/
extern I2C_HandleTypeDef MAX30102_I2C_HANDLE;

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief Initialize MAX30102 sensor
 * @retval true if successful, false otherwise
 */
bool MAX30102_Init(void);

/**
 * @brief Reset MAX30102 sensor
 * @retval true if successful, false otherwise
 */
bool MAX30102_Reset(void);

/**
 * @brief Write data to MAX30102 register
 * @param reg_addr Register address
 * @param data Data to write
 * @retval true if successful, false otherwise
 */
bool MAX30102_WriteReg(uint8_t reg_addr, uint8_t data);

/**
 * @brief Read data from MAX30102 register
 * @param reg_addr Register address
 * @param data Pointer to store read data
 * @retval true if successful, false otherwise
 */
bool MAX30102_ReadReg(uint8_t reg_addr, uint8_t *data);

/**
 * @brief Read FIFO data from MAX30102
 * @param red_led Pointer to store red LED data
 * @param ir_led Pointer to store IR LED data
 * @retval true if successful, false otherwise
 */
bool MAX30102_ReadFIFO(uint32_t *red_led, uint32_t *ir_led);

/**
 * @brief Get device ID
 * @param part_id Pointer to store part ID
 * @param rev_id Pointer to store revision ID
 * @retval true if successful, false otherwise
 */
bool MAX30102_GetDeviceID(uint8_t *part_id, uint8_t *rev_id);

/**
 * @brief Configure sample rate and pulse width
 * @param sample_rate Sample rate setting
 * @param pulse_width Pulse width setting
 * @param adc_range ADC range setting
 * @retval true if successful, false otherwise
 */
bool MAX30102_SetConfig(uint8_t sample_rate, uint8_t pulse_width, uint8_t adc_range);

/**
 * @brief Set LED current
 * @param led1_current Red LED current (0-255, ~0.2mA per step)
 * @param led2_current IR LED current (0-255, ~0.2mA per step)
 * @retval true if successful, false otherwise
 */
bool MAX30102_SetLEDCurrent(uint8_t led1_current, uint8_t led2_current);

/**
 * @brief Check if new data is available
 * @retval true if new data available, false otherwise
 */
bool MAX30102_IsDataReady(void);

#ifdef __cplusplus
}
#endif

#endif /* MAX30102_HAL_H */
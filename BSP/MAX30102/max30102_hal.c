/**
 * @file max30102_hal.c
 * @brief MAX30102 heart rate and oxygen saturation sensor driver implementation
 * @author Converted from mbed version
 * @date 2025
 */

/* Includes ------------------------------------------------------------------*/
#include "max30102_hal.h"

/* Private variables ---------------------------------------------------------*/
// I2C handle reference (should be defined in main.c or other source file)
// I2C_HandleTypeDef MAX30102_I2C_HANDLE;

/**
 * @brief Write data to MAX30102 register
 * @param reg_addr Register address to write to
 * @param data Data byte to write
 * @retval true if successful, false otherwise
 */
bool MAX30102_WriteReg(uint8_t reg_addr, uint8_t data)
{
    uint8_t tx_data[2];
    tx_data[0] = reg_addr;
    tx_data[1] = data;

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &MAX30102_I2C_HANDLE, MAX30102_I2C_WRITE_ADDR, tx_data, 2, MAX30102_I2C_TIMEOUT);
    return (status == HAL_OK);
}

/**
 * @brief Read data from MAX30102 register
 * @param reg_addr Register address to read from
 * @param data Pointer to store the read data
 * @retval true if successful, false otherwise
 */
bool MAX30102_ReadReg(uint8_t reg_addr, uint8_t *data)
{
    HAL_StatusTypeDef status;

    // Send register address
    status = HAL_I2C_Master_Transmit(&MAX30102_I2C_HANDLE, MAX30102_I2C_WRITE_ADDR, &reg_addr, 1,
                                     MAX30102_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return false;
    }

    // Read data
    status = HAL_I2C_Master_Receive(&MAX30102_I2C_HANDLE, MAX30102_I2C_READ_ADDR, data, 1,
                                    MAX30102_I2C_TIMEOUT);
    return (status == HAL_OK);
}

/**
 * @brief Reset MAX30102 sensor
 * @retval true if successful, false otherwise
 */
bool MAX30102_Reset(void)
{
    return MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, 0x40);
}

/**
 * @brief Initialize MAX30102 with default configuration
 * @retval true if successful, false otherwise
 */
bool MAX30102_Init(void)
{
    // Reset the sensor first
    if (!MAX30102_Reset()) {
        return false;
    }

    // Wait for reset to complete
    HAL_Delay(100);

    // Interrupt configuration
    if (!MAX30102_WriteReg(MAX30102_REG_INTR_ENABLE_1, 0xC0)) {  // A_FULL_EN and PPG_RDY_EN
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_INTR_ENABLE_2, 0x00)) {
        return false;
    }

    // FIFO configuration
    if (!MAX30102_WriteReg(MAX30102_REG_FIFO_WR_PTR, 0x00)) {  // FIFO write pointer reset
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_OVF_COUNTER, 0x00)) {  // FIFO overflow counter reset
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_FIFO_RD_PTR, 0x00)) {  // FIFO read pointer reset
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_FIFO_CONFIG,
                           0x0F)) {  // Sample avg = 1, FIFO rollover = false, FIFO almost full = 15
        return false;
    }

    // Mode configuration (SpO2 mode)
    if (!MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, MAX30102_MODE_SPO2)) {
        return false;
    }

    // SpO2 configuration: ADC range = 4096nA, Sample rate = 100Hz, LED pulse width = 411μs (18-bit
    // resolution)
    if (!MAX30102_WriteReg(MAX30102_REG_SPO2_CONFIG, 0x27)) {
        return false;
    }

    // LED current configuration (~7mA for both LEDs)
    if (!MAX30102_WriteReg(MAX30102_REG_LED1_PA, 0x24)) {  // Red LED current
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_LED2_PA, 0x24)) {  // IR LED current
        return false;
    }
    if (!MAX30102_WriteReg(MAX30102_REG_PILOT_PA, 0x7F)) {  // Pilot LED current (~25mA)
        return false;
    }

    return true;
}

/**
 * @brief Read FIFO data from MAX30102
 * @param red_led Pointer to store red LED data (18-bit)
 * @param ir_led Pointer to store IR LED data (18-bit)
 * @retval true if successful, false otherwise
 */
bool MAX30102_ReadFIFO(uint32_t *red_led, uint32_t *ir_led)
{
    uint8_t temp_reg;
    uint8_t fifo_data[6];
    HAL_StatusTypeDef status;

    *red_led = 0;
    *ir_led = 0;

    // Clear interrupt status registers
    MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_1, &temp_reg);
    MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_2, &temp_reg);

    // Set register pointer to FIFO data register
    uint8_t reg_addr = MAX30102_REG_FIFO_DATA;
    status = HAL_I2C_Master_Transmit(&MAX30102_I2C_HANDLE, MAX30102_I2C_WRITE_ADDR, &reg_addr, 1,
                                     MAX30102_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return false;
    }

    // Read 6 bytes of FIFO data (3 bytes RED, 3 bytes IR)
    status = HAL_I2C_Master_Receive(&MAX30102_I2C_HANDLE, MAX30102_I2C_READ_ADDR, fifo_data, 6,
                                    MAX30102_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return false;
    }

    // Parse RED LED data (first 3 bytes, MSB first)
    *red_led =
        ((uint32_t)fifo_data[0] << 16) | ((uint32_t)fifo_data[1] << 8) | ((uint32_t)fifo_data[2]);

    // Parse IR LED data (next 3 bytes, MSB first)
    *ir_led =
        ((uint32_t)fifo_data[3] << 16) | ((uint32_t)fifo_data[4] << 8) | ((uint32_t)fifo_data[5]);

    // Mask to 18-bit resolution
    *red_led &= 0x03FFFF;
    *ir_led &= 0x03FFFF;

    return true;
}

/**
 * @brief Get MAX30102 device ID
 * @param part_id Pointer to store part ID
 * @param rev_id Pointer to store revision ID
 * @retval true if successful, false otherwise
 */
bool MAX30102_GetDeviceID(uint8_t *part_id, uint8_t *rev_id)
{
    bool result = true;
    result &= MAX30102_ReadReg(MAX30102_REG_PART_ID, part_id);
    result &= MAX30102_ReadReg(MAX30102_REG_REV_ID, rev_id);
    return result;
}

/**
 * @brief Configure sample rate, pulse width and ADC range
 * @param sample_rate Sample rate setting (0-7)
 * @param pulse_width Pulse width setting (0-3)
 * @param adc_range ADC range setting (0-3)
 * @retval true if successful, false otherwise
 */
bool MAX30102_SetConfig(uint8_t sample_rate, uint8_t pulse_width, uint8_t adc_range)
{
    uint8_t config_value =
        ((adc_range & 0x03) << 5) | ((sample_rate & 0x07) << 2) | (pulse_width & 0x03);
    return MAX30102_WriteReg(MAX30102_REG_SPO2_CONFIG, config_value);
}

/**
 * @brief Set LED current levels
 * @param led1_current Red LED current (0-255, ~0.2mA per step)
 * @param led2_current IR LED current (0-255, ~0.2mA per step)
 * @retval true if successful, false otherwise
 */
bool MAX30102_SetLEDCurrent(uint8_t led1_current, uint8_t led2_current)
{
    bool result = true;
    result &= MAX30102_WriteReg(MAX30102_REG_LED1_PA, led1_current);
    result &= MAX30102_WriteReg(MAX30102_REG_LED2_PA, led2_current);
    return result;
}

/**
 * @brief Check if new data is ready by reading interrupt status
 * @retval true if new data available, false otherwise
 */
bool MAX30102_IsDataReady(void)
{
    uint8_t status;
    if (MAX30102_ReadReg(MAX30102_REG_INTR_STATUS_1, &status)) {
        return (status & 0x40) != 0;  // Check PPG_RDY bit
    }
    return false;
}
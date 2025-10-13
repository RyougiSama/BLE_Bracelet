/**
 * @file    myiic.c
 * @brief   MAX30102 I2C communication driver using STM32F103 HAL library
 * @version V3.0
 * @date    2025-10-13
 * @note    Uses HAL I2C driver with configurable handle via MAX30102_I2C_HANDLE macro
 */

#include "myiic.h"

/// @brief Initialize I2C interface for MAX30102
/// @note I2C initialization is completely handled by CubeMX generated code:
///       - MX_I2C1_Init() or MX_I2C2_Init() called from main.c
///       - GPIO configuration (alternate function, open-drain, pull-up)
///       - Clock configuration (400kHz standard mode)
///       - No manual configuration needed here
void IIC_Init(void)
{
    // Nothing to do - all initialization handled by HAL and CubeMX
    // Current configuration (from CubeMX):
    // I2C1: PB6 -> SCL, PB7 -> SDA, 400kHz
    // I2C2: PB10 -> SCL, PB11 -> SDA, 400kHz (if enabled)
}

/// @brief Write multiple bytes to I2C device using HAL library
/// @param WriteAddr Device address (7-bit address shifted left by 1)
/// @param data Pointer to data buffer
/// @param dataLength Number of bytes to write
void IIC_WriteBytes(uint8_t WriteAddr, uint8_t *data, uint8_t dataLength)
{
    HAL_I2C_Master_Transmit(&MAX30102_I2C_HANDLE, WriteAddr, data, dataLength, HAL_MAX_DELAY);
}

/// @brief Read multiple bytes from I2C device using HAL library
/// @param deviceAddr Device address (7-bit address shifted left by 1)
/// @param writeAddr Register address to read from
/// @param data Pointer to receive buffer
/// @param dataLength Number of bytes to read
void IIC_ReadBytes(uint8_t deviceAddr, uint8_t writeAddr, uint8_t *data, uint8_t dataLength)
{
    HAL_I2C_Mem_Read(&MAX30102_I2C_HANDLE, deviceAddr, writeAddr, I2C_MEMADD_SIZE_8BIT, data,
                     dataLength, HAL_MAX_DELAY);
}

/// @brief Read one byte from I2C device using HAL library
/// @param daddr Device address (7-bit address shifted left by 1)
/// @param addr Register address to read from
/// @param data Pointer to store the read byte
void IIC_Read_One_Byte(uint8_t daddr, uint8_t addr, uint8_t *data)
{
    HAL_I2C_Mem_Read(&MAX30102_I2C_HANDLE, daddr, addr, I2C_MEMADD_SIZE_8BIT, data, 1,
                     HAL_MAX_DELAY);
}

/// @brief Write one byte to I2C device using HAL library
/// @param daddr Device address (7-bit address shifted left by 1)
/// @param addr Register address to write to
/// @param data Data byte to write
void IIC_Write_One_Byte(uint8_t daddr, uint8_t addr, uint8_t data)
{
    HAL_I2C_Mem_Write(&MAX30102_I2C_HANDLE, daddr, addr, I2C_MEMADD_SIZE_8BIT, &data, 1,
                      HAL_MAX_DELAY);
}
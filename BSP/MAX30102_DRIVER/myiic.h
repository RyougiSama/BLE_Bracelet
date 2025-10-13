/**
 * @file    myiic.h
 * @brief   MAX30102 I2C communication driver header using STM32F103 HAL library
 * @version V3.0
 * @date    2025-10-13
 * @note    Uses STM32 HAL I2C driver, completely managed by CubeMX
 *          Default: I2C1 peripheral (PB6->SCL, PB7->SDA)
 *          Can be switched by changing MAX30102_I2C_HANDLE macro
 */

#ifndef __MYIIC_H
#define __MYIIC_H

#include <stdint.h>

#include "i2c.h"
#include "main.h"

/// @brief I2C handle selection for MAX30102 (configurable)
/// @note Change this macro to switch between I2C1/I2C2 if needed
#define MAX30102_I2C_HANDLE hi2c1

/// @brief MAX30102 I2C communication functions
/// @note All I2C initialization is handled by CubeMX generated code
void IIC_Init(void);
void IIC_Write_One_Byte(uint8_t daddr, uint8_t addr, uint8_t data);
void IIC_Read_One_Byte(uint8_t daddr, uint8_t addr, uint8_t *data);
void IIC_WriteBytes(uint8_t WriteAddr, uint8_t *data, uint8_t dataLength);
void IIC_ReadBytes(uint8_t deviceAddr, uint8_t writeAddr, uint8_t *data, uint8_t dataLength);

/// @note The following functions are no longer needed with HAL library:
/// - All low-level I2C timing control is handled by HAL
/// - No need for manual start/stop/ack/nack operations
/// - GPIO configuration is completely managed by CubeMX
/// - Clock configuration is handled in system initialization

#endif
#ifndef __MPU6050_H
#define __MPU6050_H

#include "i2c.h"

extern float g_ax, g_ay, g_az;  // 加速度，单位g
extern float g_gx, g_gy, g_gz;  // 角速度，单位°/s
extern float g_temp;            // 温度，单位°C

HAL_StatusTypeDef MPU6050_Init(void);
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Read_Temp(void);
void MPU6050_Read_All(void);

#endif

#include "mpu6050.h"


// 原始数据
int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
int16_t Gyro_X_RAW, Gyro_Y_RAW, Gyro_Z_RAW;
int16_t Temp_RAW;

// 转换后的物理量
float g_ax, g_ay, g_az;  // 加速度，单位g
float g_gx, g_gy, g_gz;  // 角速度，单位°/s
float g_temp;            // 温度，单位°C

// 使用的I2C句柄
#define MPU6050_HI2C hi2c2

/* MPU6050 I2C设备地址定义 */
#define MPU6050_ADDR (0x68 << 1)  // 若AD0接地，7位地址0x68，左移1位得到8位地址0xD0
#define WHO_AM_I_REG 0x75         // WHO_AM_I寄存器地址，默认值0x68
#define PWR_MGMT_1_REG 0x6B       // 电源管理寄存器1
#define SMPLRT_DIV_REG 0x19       // 采样率分频寄存器
#define CONFIG_REG 0x1A           // 配置寄存器（含DLPF设置）
#define GYRO_CONFIG_REG 0x1B      // 陀螺仪配置寄存器
#define ACCEL_CONFIG_REG 0x1C     // 加速度计配置寄存器

/* 初始化 MPU6050 */
HAL_StatusTypeDef MPU6050_Init(void)
{
    uint8_t check, data;
    HAL_StatusTypeDef res;
    // 1. 读取 WHO_AM_I 寄存器，检查设备ID是否正确 (0x68)
    res = HAL_I2C_Mem_Read(&MPU6050_HI2C, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);
    if (res != HAL_OK || check != 0x68) {
        return HAL_ERROR;  // 通信失败或ID不符
    }
    // 2. 解除休眠，将 PWR_MGMT_1 寄存器写0
    data = 0x00;
    HAL_I2C_Mem_Write(&MPU6050_HI2C, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data, 1, 100);
    HAL_Delay(10);  // 小延迟，等待芯片唤醒稳定
    // 3. 设置采样率分频器 SMPLRT_DIV (比如设置成7获得1kHz采样率)
    data = 0x07;
    HAL_I2C_Mem_Write(&MPU6050_HI2C, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &data, 1, 100);
    // 4. 配置DLPF，在CONFIG寄存器中设置数字低通滤波器 (例如0x03,Accel带宽44Hz)
    data = 0x03;
    HAL_I2C_Mem_Write(&MPU6050_HI2C, MPU6050_ADDR, CONFIG_REG, 1, &data, 1, 100);
    // 5. 配置陀螺仪满量程范围 ±250°/s (0x00) 和加速度计满量程范围 ±2g (0x00)
    data = 0x00;
    HAL_I2C_Mem_Write(&MPU6050_HI2C, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, 100);
    data = 0x00;
    HAL_I2C_Mem_Write(&MPU6050_HI2C, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, 100);
    return HAL_OK;
}

// 读取加速度计三轴原始数据
void MPU6050_Read_Accel(void)
{
    uint8_t buf[6];
    HAL_I2C_Mem_Read(&MPU6050_HI2C, MPU6050_ADDR, 0x3B, 1, buf, 6, 100);
    // 拼接高低字节为 16 位有符号值
    Accel_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
    Accel_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
    Accel_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);
}

// 读取陀螺仪三轴原始数据
void MPU6050_Read_Gyro(void)
{
    uint8_t buf[6];
    HAL_I2C_Mem_Read(&MPU6050_HI2C, MPU6050_ADDR, 0x43, 1, buf, 6, 100);
    Gyro_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
    Gyro_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
    Gyro_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);
}

// 读取温度原始数据
void MPU6050_Read_Temp(void)
{
    uint8_t buf[2];
    HAL_I2C_Mem_Read(&MPU6050_HI2C, MPU6050_ADDR, 0x41, 1, buf, 2, 100);
    Temp_RAW = (int16_t)(buf[0] << 8 | buf[1]);
}

// 读取所有传感器数据
void MPU6050_Read_All(void)
{
    uint8_t buf[14];
    // 一次性读取 加速度(6)+温度(2)+陀螺仪(6) 共14字节
    HAL_I2C_Mem_Read(&MPU6050_HI2C, MPU6050_ADDR, 0x3B, 1, buf, 14, 100);
    // 加速度
    Accel_X_RAW = (int16_t)(buf[0] << 8 | buf[1]);
    Accel_Y_RAW = (int16_t)(buf[2] << 8 | buf[3]);
    Accel_Z_RAW = (int16_t)(buf[4] << 8 | buf[5]);
    // 温度
    Temp_RAW = (int16_t)(buf[6] << 8 | buf[7]);
    // 陀螺仪
    Gyro_X_RAW = (int16_t)(buf[8] << 8 | buf[9]);
    Gyro_Y_RAW = (int16_t)(buf[10] << 8 | buf[11]);
    Gyro_Z_RAW = (int16_t)(buf[12] << 8 | buf[13]);
    // 转换为物理量
    g_ax = Accel_X_RAW / 16384.0f;
    g_ay = Accel_Y_RAW / 16384.0f;
    g_az = Accel_Z_RAW / 16384.0f;
    g_gx = Gyro_X_RAW / 131.0f;
    g_gy = Gyro_Y_RAW / 131.0f;
    g_gz = Gyro_Z_RAW / 131.0f;
    g_temp = Temp_RAW / 340.0f + 36.53f;
}

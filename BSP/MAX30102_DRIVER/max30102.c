/**
 * @file    max30102.c
 * @brief   MAX30102 heart rate and SpO2 sensor driver using STM32F103 HAL library
 * @version V3.0
 * @date    2025-10-13
 * @note    Uses HAL I2C with configurable handle, all I2C config handled by CubeMX
 */

#include "max30102.h"

#include "myiic.h"

/// @brief Write data to MAX30102 register using HAL I2C
/// @param Register_Address Target register address
/// @param Word_Data Data byte to write
/// @return 1 if successful, 0 if failed
uint8_t max30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data)
{
    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Write(&MAX30102_I2C_HANDLE, max30102_WR_address, Register_Address,
                          I2C_MEMADD_SIZE_8BIT, &Word_Data, 1, HAL_MAX_DELAY);
    return (status == HAL_OK) ? 1 : 0;
}

/// @brief Read data from MAX30102 register using HAL I2C
/// @param Register_Address Target register address
/// @return Register value if successful, 0 if failed
uint8_t max30102_Bus_Read(uint8_t Register_Address)
{
    uint8_t data = 0;
    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Read(&MAX30102_I2C_HANDLE, max30102_WR_address, Register_Address,
                         I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    return (status == HAL_OK) ? data : 0;
}  /// @brief Read multiple 16-bit words from MAX30102 FIFO using HAL I2C
/// @param Register_Address Target register address (typically REG_FIFO_DATA)
/// @param Word_Data 2D array to store [count][2] words (Red and IR values)
/// @param count Number of samples to read
void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count)
{
    uint8_t buffer[count * 4];  // Each sample has 4 bytes (2 x 16-bit values)

    // Read all data at once using HAL I2C
    if (HAL_I2C_Mem_Read(&MAX30102_I2C_HANDLE, max30102_WR_address, Register_Address,
                         I2C_MEMADD_SIZE_8BIT, buffer, count * 4, HAL_MAX_DELAY) == HAL_OK) {
        // Parse the received data
        for (uint8_t i = 0; i < count; i++) {
            Word_Data[i][0] = ((uint16_t)buffer[i * 4] << 8) | buffer[i * 4 + 1];  // Red LED data
            Word_Data[i][1] =
                ((uint16_t)buffer[i * 4 + 2] << 8) | buffer[i * 4 + 3];  // IR LED data
        }
    }
}

/// @brief Read 6 bytes from MAX30102 FIFO using HAL I2C
/// @param Register_Address Target register address (typically REG_FIFO_DATA)
/// @param Data Buffer to store 6 bytes of data
void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data)
{
    // Clear interrupt status registers
    max30102_Bus_Read(REG_INTR_STATUS_1);
    max30102_Bus_Read(REG_INTR_STATUS_2);

    // Read 6 bytes from FIFO data register
    HAL_I2C_Mem_Read(&MAX30102_I2C_HANDLE, max30102_WR_address, Register_Address,
                     I2C_MEMADD_SIZE_8BIT, Data, 6, HAL_MAX_DELAY);

    //  uint8_t i;
    //  uint8_t fifo_wr_ptr;
    //  uint8_t firo_rd_ptr;
    //  uint8_t number_tp_read;
    //  //Get the FIFO_WR_PTR
    //  fifo_wr_ptr = max30102_Bus_Read(REG_FIFO_WR_PTR);
    //  //Get the FIFO_RD_PTR
    //  firo_rd_ptr = max30102_Bus_Read(REG_FIFO_RD_PTR);
    //
    //  number_tp_read = fifo_wr_ptr - firo_rd_ptr;
    //
    //  //for(i=0;i<number_tp_read;i++){
    //  if(number_tp_read>0){
    //      IIC_ReadBytes(max30102_WR_address,REG_FIFO_DATA,Data,6);
    //  }

    // max30102_Bus_Write(REG_FIFO_RD_PTR,fifo_wr_ptr);
}

/// @brief Initialize MAX30102 sensor using HAL library
void max30102_init(void)
{
    // GPIO configuration for MAX30102_INT pin is handled by CubeMX generated code
    // INT pin should be configured as GPIO input with pull-up in CubeMX:
    // Label: MAX30102_INT, Pin: PB13, Mode: GPIO_Input, Pull-up: Yes

    IIC_Init();

    max30102_reset();

    //  max30102_Bus_Write(REG_MODE_CONFIG, 0x0b);  //mode configuration : temp_en[3] MODE[2:0]=010
    //  HR only enabled    011 SP02 enabled max30102_Bus_Write(REG_INTR_STATUS_2, 0xF0); //open all
    //  of interrupt max30102_Bus_Write(REG_INTR_STATUS_1, 0x00); //all interrupt clear
    //  max30102_Bus_Write(REG_INTR_ENABLE_2, 0x02); //DIE_TEMP_RDY_EN
    //  max30102_Bus_Write(REG_TEMP_CONFIG, 0x01); //SET   TEMP_EN

    //  max30102_Bus_Write(REG_SPO2_CONFIG, 0x47); //SPO2_SR[4:2]=001  100 per second LED_PW[1:0]=11
    //  16BITS

    //  max30102_Bus_Write(REG_LED1_PA, 0x47);
    //  max30102_Bus_Write(REG_LED2_PA, 0x47);

    max30102_Bus_Write(REG_INTR_ENABLE_1, 0xc0);  // INTR setting
    max30102_Bus_Write(REG_INTR_ENABLE_2, 0x00);
    max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00);  // FIFO_WR_PTR[4:0]
    max30102_Bus_Write(REG_OVF_COUNTER, 0x00);  // OVF_COUNTER[4:0]
    max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00);  // FIFO_RD_PTR[4:0]
    max30102_Bus_Write(REG_FIFO_CONFIG,
                       0x0f);  // sample avg = 1, fifo rollover=false, fifo almost full = 17
    max30102_Bus_Write(REG_MODE_CONFIG,
                       0x03);  // 0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
    max30102_Bus_Write(
        REG_SPO2_CONFIG,
        0x27);  // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
    max30102_Bus_Write(REG_LED1_PA, 0x24);   // Choose value for ~ 7mA for LED1
    max30102_Bus_Write(REG_LED2_PA, 0x24);   // Choose value for ~ 7mA for LED2
    max30102_Bus_Write(REG_PILOT_PA, 0x7f);  // Choose value for ~ 25mA for Pilot LED

    //  // Interrupt Enable 1 Register. Set PPG_RDY_EN (data available in FIFO)
    //  max30102_Bus_Write(0x2, 1<<6);

    //  // FIFO configuration register
    //  // SMP_AVE: 16 samples averaged per FIFO sample
    //  // FIFO_ROLLOVER_EN=1
    //  //max30102_Bus_Write(0x8,  1<<4);
    //  max30102_Bus_Write(0x8, (0<<5) | 1<<4);

    //  // Mode Configuration Register
    //  // SPO2 mode
    //  max30102_Bus_Write(0x9, 3);

    //  // SPO2 Configuration Register
    //  max30102_Bus_Write(0xa,
    //          (3<<5)  // SPO2_ADC_RGE 2 = full scale 8192 nA (LSB size 31.25pA); 3 = 16384nA
    //          | (1<<2) // sample rate: 0 = 50sps; 1 = 100sps; 2 = 200sps
    //          | (3<<0) // LED_PW 3 = 411μs, ADC resolution 18 bits
    //  );

    //  // LED1 (red) power (0 = 0mA; 255 = 50mA)
    //  max30102_Bus_Write(0xc, 0xb0);

    //  // LED (IR) power
    //  max30102_Bus_Write(0xd, 0xa0);
}

void max30102_reset(void)
{
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
}

void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
    //  char ach_i2c_data[2];
    //  ach_i2c_data[0]=uch_addr;
    //  ach_i2c_data[1]=uch_data;
    //
    //  IIC_WriteBytes(I2C_WRITE_ADDR, ach_i2c_data, 2);
    IIC_Write_One_Byte(I2C_WRITE_ADDR, uch_addr, uch_data);
}

void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
{
    //  char ch_i2c_data;
    //  ch_i2c_data=uch_addr;
    //  IIC_WriteBytes(I2C_WRITE_ADDR, &ch_i2c_data, 1);
    //
    //  i2c.read(I2C_READ_ADDR, &ch_i2c_data, 1);
    //
    //   *puch_data=(uint8_t) ch_i2c_data;
    IIC_Read_One_Byte(I2C_WRITE_ADDR, uch_addr, puch_data);
}

void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
{
    uint32_t un_temp;
    unsigned char uch_temp;
    char ach_i2c_data[6];
    *pun_red_led = 0;
    *pun_ir_led = 0;

    // read and clear status register
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
    maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);

    IIC_ReadBytes(I2C_WRITE_ADDR, REG_FIFO_DATA, (uint8_t *)ach_i2c_data, 6);

    un_temp = (unsigned char)ach_i2c_data[0];
    un_temp <<= 16;
    *pun_red_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[1];
    un_temp <<= 8;
    *pun_red_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[2];
    *pun_red_led += un_temp;

    un_temp = (unsigned char)ach_i2c_data[3];
    un_temp <<= 16;
    *pun_ir_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[4];
    un_temp <<= 8;
    *pun_ir_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[5];
    *pun_ir_led += un_temp;
    *pun_red_led &= 0x03FFFF;  // Mask MSB [23:18]
    *pun_ir_led &= 0x03FFFF;   // Mask MSB [23:18]
}
#include "blood_measure_task.h"

int32_t g_heart_rate, g_spo2;
bool g_is_hr_valid, g_is_spo2_valid;

// 血氧心率测量任务
void Task_BloodMeasure(void)
{
    MAX30102_Task_Handler();
}

// 更新血氧心率数据
void Blood_Data_Update(void)
{
    MAX30102_Get_Results(&g_heart_rate, &g_spo2, &g_is_hr_valid, &g_is_spo2_valid);
}

// 外部中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MAX30102_INT_Pin) {
        // MAX30102中断处理
        MAX30102_DataReady_ISR();
    }
}

#include "blood_measure_task.h"

#include "blood.h"

// 初始化血氧心率测量任务
void BloodMeasure_Init(void)
{
    max30102_reset();
    max30102_config();
}

// 血氧心率测量任务
void Task_BloodMeasure(void)
{
    blood_loop();
}

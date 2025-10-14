#include "step_count.h"

// 定时器中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        // 50ms执行一次TIM6中断
        Timer_Handler_StepCount();
    }
}
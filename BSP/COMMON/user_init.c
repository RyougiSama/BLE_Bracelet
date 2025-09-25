#include "user_init.h"
#include "app_tasks.h"

/**
 * @brief 用户自定义初始化函数
 *
 */
void User_Init(void)
{
    AppTasks_Init();

    while (1) {
        HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
        HAL_Delay(1000);
    }
}

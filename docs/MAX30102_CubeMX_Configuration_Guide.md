# MAX30102外部中断配置指南 - CubeMX设置

## 概述
本文档说明如何在STM32CubeMX中配置MAX30102传感器的外部中断功能，将中断引脚从PB5修改为PB4。

## 硬件连接
- **I2C通信**：PB8(SCL)，PB9(SDA)
- **中断引脚**：PB4 (MAX30102的INT引脚)
- **电源**：3.3V供电

## CubeMX配置步骤

### 1. 打开CubeMX项目
打开您的BLE_Bracelet项目的`.ioc`文件

### 2. 配置GPIO引脚
1. 在Pinout视图中找到**PB4**引脚
2. 点击PB4引脚，选择**GPIO_EXTI4**
3. 确保PB4引脚被标记为外部中断引脚

### 3. 配置GPIO参数
在左侧面板的**GPIO**配置中：

#### PB4 配置
- **GPIO mode**: External Interrupt Mode with Falling edge trigger detection
- **GPIO Pull-up/Pull-down**: Pull-up
- **Maximum output speed**: Low (不适用于输入)
- **User Label**: MAX30102_INT (可选)

### 4. 配置NVIC中断
在**NVIC**配置页面：
1. 启用**EXTI line4 interrupt**
2. 设置优先级：
   - **Preemption Priority**: 2
   - **Sub Priority**: 0

### 5. 生成代码
1. 点击**GENERATE CODE**
2. 确保选择了正确的IDE (例如：MDK-ARM V5)
3. 生成代码

## 代码集成

### 1. 中断服务程序
在`stm32f1xx_it.c`文件中，CubeMX会自动生成`EXTI4_IRQHandler`函数：

```c
/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */

  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
  /* USER CODE BEGIN EXTI4_IRQn 1 */

  /* USER CODE END EXTI4_IRQn 1 */
}
```

### 2. 中断回调函数
在您的应用代码中实现GPIO外部中断回调函数：

```c
/**
  * @brief EXTI line detection callback.
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_4) {
        // MAX30102中断处理
        MAX30102_DataReady_ISR();
    }
}
```

### 3. 在main.c中添加回调函数
如果尚未添加，在`main.c`的USER CODE区域添加上述回调函数。

## 验证配置

### 1. 编译检查
- 编译项目确保没有错误
- 检查是否生成了正确的中断向量

### 2. 功能测试
1. 连接MAX30102传感器到指定引脚
2. 烧录程序到STM32
3. 测试中断是否正常触发

### 3. 调试建议
- 使用示波器检查PB4引脚的中断信号
- 在中断服务程序中添加LED指示验证中断触发
- 检查上拉电阻是否正确配置

## 注意事项

1. **引脚冲突检查**：确保PB4没有被其他功能占用
2. **电压兼容性**：MAX30102使用3.3V逻辑电平，与STM32F103兼容
3. **中断优先级**：确保中断优先级设置合理，不会影响其他关键中断
4. **去抖动处理**：如果需要，在软件中实现中断去抖动
5. **功耗考虑**：外部中断会唤醒休眠模式，需要在低功耗设计中考虑

## 故障排除

### 常见问题
1. **中断不触发**
   - 检查硬件连接
   - 验证上拉电阻配置
   - 确认中断边沿触发模式

2. **编译错误**
   - 检查NVIC配置是否启用
   - 确认生成的代码中包含中断处理函数

3. **中断频繁触发**
   - 检查硬件干扰
   - 考虑添加软件去抖动
   - 检查信号完整性

### 调试步骤
1. 使用万用表检查引脚电压
2. 在中断服务程序中设置断点
3. 使用逻辑分析仪监控I2C通信和中断信号

## 相关文件
- `max30102_config.h` - 配置定义已更新为PB4
- `max30102_example.c` - 中断处理代码已更新
- `max30102_example.h` - 函数声明已更新

---
**注意**：本配置适用于STM32F103系列微控制器。其他STM32系列可能需要稍作调整。
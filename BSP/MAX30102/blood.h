/**
 * ************************************************************************
 *
 * @file blood.h
 * @author zxr
 * @brief
 *
 * ************************************************************************
 * @copyright Copyright (c) 2024 zxr
 * ************************************************************************
 */
#ifndef __BLOOD_H
#define __BLOOD_H

#include <stdint.h>

#include "algorithm.h"
#include "main.h"
#include "math.h"
#include "max30102.h"

extern int32_t g_heart_rate;  // 定义心率
extern float g_SpO2;          // 定义血氧饱和度

void blood_data_translate(void);
void blood_data_update(void);
void blood_loop(void);

#endif

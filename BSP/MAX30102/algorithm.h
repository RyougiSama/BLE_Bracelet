/**
 * ************************************************************************
 *
 * @file algorithm.h
 * @author zxr
 * @brief
 *
 * ************************************************************************
 * @copyright Copyright (c) 2024 zxr
 * ************************************************************************
 */
#ifndef __ALGORITHM_H
#define __ALGORITHM_H

#include <stdint.h>

#define FFT_N 1024     // 定义傅里叶变换的点数
#define START_INDEX 8  // 低频过滤阈值

struct compx  // 定义一个复数结构
{
    float real;
    float imag;
};

typedef struct  // 定义一个直流滤波器结构体
{
    float w;
    int32_t init;
    float a;
} DC_FilterData;  // 用于存储直流滤波器的参数

typedef struct  // 定义一个带宽滤波器结构体
{
    float v0;
    float v1;
} BW_FilterData;  // 用于存储带宽滤波器的参数

double my_floor(double x);

double my_fmod(double x, double y);

double x_sin(double x);

double x_cos(double x);

int32_t q_sqrt(int32_t a);

struct compx complex_multiply(struct compx a, struct compx b);

void fft(struct compx *xin);

int32_t find_max_num_index(struct compx *data, int32_t count);
int32_t dc_filter(int32_t input, DC_FilterData *df);
int32_t bw_filter(int32_t input, BW_FilterData *bw);

#endif

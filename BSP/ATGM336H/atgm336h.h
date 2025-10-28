#ifndef __ATGM336H_H
#define __ATGM336H_H

#include "stdbool.h"
#include "usart.h"

#define ATGM336H_USART_HANDLER huart3

#define USART_REC_LEN 200  // 定义最大接收字节数 200
#define EN_USART1_RX 1     // 使能（1）/禁止（0）串口1接收

// 定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2

typedef struct SaveData {
    char GPS_Buffer[GPS_Buffer_Length];
    char isGetData;                    // 是否获取到GPS数据
    char isParseData;                  // 是否解析完成
    char UTCTime[UTCTime_Length];      // UTC时间
    char latitude[latitude_Length];    // 纬度
    char N_S[N_S_Length];              // N/S
    char longitude[longitude_Length];  // 经度
    char E_W[E_W_Length];              // E/W
    char isUsefull;                    // 定位信息是否有效
} _SaveData;

// 经纬度数据
typedef struct _LatitudeAndLongitude_s {
    float latitude;   // 纬度
    float longitude;  // 经度

    char N_S;  // 北南
    char E_W;  // 东西
} LatitudeAndLongitude_s;

extern char rxdatabufer;
extern uint16_t point1;
extern _SaveData Save_Data;
extern LatitudeAndLongitude_s g_LatAndLongData;

// 放入串口接收中断中
void atgm336h_UART_RxCpltCallback(UART_HandleTypeDef* huart);

// 初始化
void atgm336h_init(void);

// 清除结构体数据
void clrStruct(void);

// 解包函数
void parseGpsBuffer(void);

// 打印函数
void printGpsBuffer(void);

#endif  // __ATGM336H_H

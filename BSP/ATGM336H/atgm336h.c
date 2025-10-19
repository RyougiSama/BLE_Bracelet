#include "atgm336h.h"

#include "stdio.h"
#include "string.h"

char rxdatabufer;
uint16_t point1 = 0;

_SaveData Save_Data;
LatitudeAndLongitude_s g_LatAndLongData = {.E_W = 0, .N_S = 0, .latitude = 0.0, .longitude = 0.0};

// 串口1中断服务程序
// 注意,读取USARTx->SR能避免莫名其妙的错误
char USART_RX_BUF[USART_REC_LEN];  // 接收缓冲,最大USART_REC_LEN个字节.
uint8_t uart_A_RX_Buff;

// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
uint16_t USART_RX_STA = 0;  // 接收状态标记

void atgm336h_init(void) {
    clrStruct();
    HAL_UART_Receive_IT(&ATGM336H_USART_HANDLER, &uart_A_RX_Buff, 1);
}

void atgm336h_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART1) {
        //		printf("%c", uart_A_RX_Buff);
        if (uart_A_RX_Buff == '$') {
            point1 = 0;
        }
        USART_RX_BUF[point1++] = uart_A_RX_Buff;

        if (USART_RX_BUF[0] == '$' && USART_RX_BUF[4] == 'M' &&
            USART_RX_BUF[5] == 'C')  // 确定是否收到"GPRMC/GNRMC"这一帧数据
        {
            if (uart_A_RX_Buff == '\n') {
                memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);  // 清空
                memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, point1);  // 保存数据
                Save_Data.isGetData = true;
                point1 = 0;
                memset(USART_RX_BUF, 0, USART_REC_LEN);  // 清空
            }
        }
        if (point1 >= USART_REC_LEN) {
            point1 = USART_REC_LEN;
        }

        HAL_UART_Receive_IT(&ATGM336H_USART_HANDLER, &uart_A_RX_Buff, 1);
    }
}

uint8_t Hand(char* a)  // 串口命令识别函数
{
    if (strstr(USART_RX_BUF, a) != NULL)
        return 1;
    else
        return 0;
}

void CLR_Buf(void)  // 串口缓存清理
{
    memset(USART_RX_BUF, 0, USART_REC_LEN);  // 清空
    point1 = 0;
}

void clrStruct(void) {
    Save_Data.isGetData = false;
    Save_Data.isParseData = false;
    Save_Data.isUsefull = false;
    memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);  // 清空
    memset(Save_Data.UTCTime, 0, UTCTime_Length);
    memset(Save_Data.latitude, 0, latitude_Length);
    memset(Save_Data.N_S, 0, N_S_Length);
    memset(Save_Data.longitude, 0, longitude_Length);
    memset(Save_Data.E_W, 0, E_W_Length);
}

void errorLog(int num) {
    while (1) {
        printf("ERROR%d\r\n", num);
    }
}

void parseGpsBuffer(void) {
    char* subString;
    char* subStringNext;
    char i = 0;

    uint16_t Number = 0, Integer = 0, Decimal = 0;

    if (Save_Data.isGetData) {
        Save_Data.isGetData = false;
        // printf("**************\r\n");
        // printf("%s\r\n", Save_Data.GPS_Buffer);

        for (i = 0; i <= 6; i++) {
            if (i == 0) {
                if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
                    errorLog(1);  // 解析错误
            } else {
                subString++;
                if ((subStringNext = strstr(subString, ",")) != NULL) {
                    char usefullBuffer[2];
                    switch (i) {
                        case 1:
                            memcpy(Save_Data.UTCTime, subString, subStringNext - subString);
                            break;  // 获取UTC时间
                        case 2:
                            memcpy(usefullBuffer, subString, subStringNext - subString);
                            break;  // 获取UTC时间
                        case 3:
                            memcpy(Save_Data.latitude, subString, subStringNext - subString);
                            break;  // 获取纬度信息
                        case 4:
                            memcpy(Save_Data.N_S, subString, subStringNext - subString);
                            break;  // 获取N/S
                        case 5:
                            memcpy(Save_Data.longitude, subString, subStringNext - subString);
                            break;  // 获取经度信息
                        case 6:
                            memcpy(Save_Data.E_W, subString, subStringNext - subString);
                            break;  // 获取E/W

                        default:
                            break;
                    }
                    subString = subStringNext;
                    Save_Data.isParseData = true;
                    if (usefullBuffer[0] == 'A')
                        Save_Data.isUsefull = true;
                    else if (usefullBuffer[0] == 'V')
                        Save_Data.isUsefull = false;
                } else {
                    errorLog(2);  // 解析错误
                }
            }
        }

        if (Save_Data.isParseData) {
            if (Save_Data.isUsefull) {
                // 获取 N/S 和 E/W
                g_LatAndLongData.N_S = Save_Data.N_S[0];
                g_LatAndLongData.E_W = Save_Data.E_W[0];

                // 获取纬度
                for (uint8_t i = 0; i < 9; i++) {
                    if (i < 2) {
                        Number *= 10;
                        Number += Save_Data.latitude[i] - '0';
                    } else if (i < 4) {
                        Integer *= 10;
                        Integer += Save_Data.latitude[i] - '0';
                    } else if (i == 4)
                        ;
                    else if (i < 9) {
                        Decimal *= 10;
                        Decimal += Save_Data.latitude[i] - '0';
                    }
                }
                g_LatAndLongData.latitude =
                    1.0 * Number + (1.0 * Integer + 1.0 * Decimal / 10000) / 60;

                Number = 0;
                Integer = 0;
                Decimal = 0;

                // 获取经度
                for (uint8_t i = 0; i < 10; i++) {
                    if (i < 3) {
                        Number *= 10;
                        Number += Save_Data.longitude[i] - '0';
                    } else if (i < 5) {
                        Integer *= 10;
                        Integer += Save_Data.longitude[i] - '0';
                    } else if (i == 5)
                        ;
                    else if (i < 10) {
                        Decimal *= 10;
                        Decimal += Save_Data.longitude[i] - '0';
                    }
                }
                g_LatAndLongData.longitude =
                    1.0 * Number + (1.0 * Integer + 1.0 * Decimal / 10000) / 60;
            }
        }
    }
}

void printGpsBuffer(void) {
    if (Save_Data.isParseData) {
        Save_Data.isParseData = false;

        printf("Save_Data.UTCTime = %s\r\n", Save_Data.UTCTime);

        if (Save_Data.isUsefull) {
            Save_Data.isUsefull = false;
            printf("Save_Data.latitude = %s\r\n", Save_Data.latitude);
            printf("Save_Data.N_S = %s", Save_Data.N_S);
            printf("Save_Data.longitude = %s", Save_Data.longitude);
            printf("Save_Data.E_W = %s\r\n", Save_Data.E_W);

            printf("latitude: %c,%.4f\r\n", g_LatAndLongData.N_S, g_LatAndLongData.latitude);
            printf("longitude: %c,%.4f\r\n", g_LatAndLongData.E_W, g_LatAndLongData.longitude);
        } else {
            printf("GPS DATA is not usefull!\r\n");
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    atgm336h_UART_RxCpltCallback(huart);
}

// ATGM336H 使用示例
void ATG336H_Example(void) {
    atgm336h_init();

    while (1) {
        parseGpsBuffer();
        printGpsBuffer();

        // if (Save_Data.isParseData) {
        //     Save_Data.isParseData = false;
        //     if (Save_Data.isUsefull) {
        //         Save_Data.isUsefull = false;
        //         printf("latitude: %c,%.4f\r\n", g_LatAndLongData.N_S, g_LatAndLongData.latitude);
        //         printf("longitude: %c,%.4f\r\n", g_LatAndLongData.E_W, g_LatAndLongData.longitude);
        //     } else {
        //         printf("GPS DATA is not usefull!\r\n");
        //     }
        // }
        HAL_Delay(30);
    }
}

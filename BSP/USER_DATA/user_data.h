#ifndef __USER_DATA_H
#define __USER_DATA_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char time[20];
    int32_t hr;
    int32_t spo2;
} UserHealthData;

void UserData_UpdateHealth(void);

#endif

#ifndef __PRINT_H__
#define __PRINT_H__

#include "global.h"

extern SemaphoreHandle_t printTempHumidSemaphore;

void printTH(void* pvParameters);
void printCondition(void* pvParameters);

#endif

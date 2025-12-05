#ifndef __WATERSENSOR_H__
#define __WATERSENSOR_H__

#include "global.h"

#define SIGNAL_PIN 2
#define POWER_PIN 38

void waterSensing(void *pvParamters);
extern xQueueHandle waterValueQueue;
extern SemaphoreHandle_t xWaterSemaphore;

#endif
#ifndef __FANCONTROL_H__
#define __FANCONTROL_H__

#include "global.h"

#define SIG_PIN 13
const int pwmPin = 13;
const int pwmChannel = 0;
const int pwmFreq = 25000;
const int pwmResolution = 8;

extern int lastSpeed;
extern QueueHandle_t fanSpeedQueue;

void fanDrive(void* pvParameters);

#endif
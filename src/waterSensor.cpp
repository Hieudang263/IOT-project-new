#include "waterSensor.h"

xQueueHandle waterValueQueue = xQueueCreate(1, sizeof(int));
SemaphoreHandle_t xWaterSemaphore = xSemaphoreCreateBinary();

void waterSensing(void* pvParameters){
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);

    while(1){
        digitalWrite(POWER_PIN, HIGH);
        vTaskDelay(10);

        int value = analogRead(SIGNAL_PIN);
        xQueueOverwrite(waterValueQueue, &value);
        xSemaphoreGive(xWaterSemaphore);

        digitalWrite(POWER_PIN, LOW);
        vTaskDelay(1000);   
    }
}
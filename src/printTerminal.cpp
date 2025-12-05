#include "printTerminal.h"

SemaphoreHandle_t printTempHumidSemaphore = xSemaphoreCreateBinary();

void printTH(void* pvParameters){
    while(1){    
        TempHumid receiver;
        xQueuePeek(TempHumidQueue, &receiver, 100);

        if(receiver.temperature == 0) continue;
        Serial.print("Temperature: ");
        Serial.print(receiver.temperature);
        Serial.print("°C Humidity: ");
        Serial.print(receiver.humidity);
        Serial.println("% ");

        xSemaphoreGive(printTempHumidSemaphore);
        vTaskDelay(5000);
    }
}

void printCondition(void* pvParameters){
    while(1){
        while(1){
            if(xSemaphoreTake(printTempHumidSemaphore, portMAX_DELAY))
                break;
        }
        TempHumid receiver;
        xQueuePeek(TempHumidQueue, &receiver, 100);

        if(receiver.temperature == 0){
                Serial.println("Temperature initializing...");
        }
        else if(receiver.temperature > 25){
                Serial.println("Satisfactory Temperature");
        } 
        else if(receiver.temperature > 20){
                Serial.println("Cool Temperature");
        } 
        else if(receiver.temperature > 15){
                Serial.println("Freezing Temperature");
        }
    }
}
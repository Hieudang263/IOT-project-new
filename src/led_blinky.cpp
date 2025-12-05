#include "led_blinky.h"

//LED_PIN is defined Pin_48 LED_GPIO
void led_blinky(void* pvParameter){
    pinMode(LED_GPIO, OUTPUT);
    uint8_t ledState = 0;
    vTaskDelay(500);
    while(1){
        TempHumid receiver;
        xQueuePeek(TempHumidQueue, &receiver, 100);
        ledState = (ledState == 0? 1 : 0);
        if(ledState == 0) digitalWrite(LED_GPIO, LOW);
        else digitalWrite(LED_GPIO, HIGH);
        if(receiver.temperature == 0){
            vTaskDelay(1000);
        }
        else if(receiver.temperature > 25){
            vTaskDelay(200);
        }
        else if(receiver.temperature > 20){
            vTaskDelay(500);
        }
        else if(receiver.temperature > 15){
            vTaskDelay(1000);
        }
    }
}
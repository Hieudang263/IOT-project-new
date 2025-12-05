#include "fanControl.h"

int lastSpeed = 0;
QueueHandle_t fanSpeedQueue = xQueueCreate(1, sizeof(int));

void fanDrive(void* pvParameters){
    TempHumid th;
    uint8_t speed;
    ledcSetup(pwmChannel, pwmFreq, pwmResolution);
    ledcAttachPin(pwmPin, pwmChannel);

    while(1){
        xQueuePeek(TempHumidQueue, &th, 100);
        int cap;

        if(th.temperature > 35) cap = 255;
        else cap = (th.temperature * 255)/35;

        if(lastSpeed < cap){
            for(speed = lastSpeed; speed < cap; speed++){
                ledcWrite(pwmChannel, speed);
                vTaskDelay(10);
            }
        }
        else{
            for(speed = lastSpeed; speed > cap; speed--){
                ledcWrite(pwmChannel, speed);
                vTaskDelay(10);
            }
        }
        xQueueOverwrite(fanSpeedQueue, &cap);
        lastSpeed = cap;
        vTaskDelay(500);
    }
}

// void fanDrive(void* pvParameters){
//     TempHumid th;
//     uint8_t speed;
//     ledcSetup(pwmChannel, pwmFreq, pwmResolution);
//     ledcAttachPin(pwmPin, pwmChannel);

//     while(1){
//         xQueuePeek(TempHumidQueue, &th, 100);
//         int cap;

//         cap = (th.humidity * 255)/100;

//         if(lastSpeed < cap){
//             for(speed = lastSpeed; speed < cap; speed++){
//                 ledcWrite(pwmChannel, speed);
//                 vTaskDelay(10);
//             }
//         }
//         else{
//             for(speed = lastSpeed; speed > cap; speed--){
//                 ledcWrite(pwmChannel, speed);
//                 vTaskDelay(10);
//             }
//         }
//         lastSpeed = cap;
//         vTaskDelay(500);
//     }
// }
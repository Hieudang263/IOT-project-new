#include <Arduino.h>
#include <WiFi.h>
#include "coreiot.h"
#include "config_coreiot.h"
#include "global.h"

void task_mqtt(void *pv) {
    Serial.println("=== MQTT task start ===");
    Serial.println("⏳ Waiting for WiFi connection...");
    
    if (xBinarySemaphoreInternet != NULL) {
        xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY);
        Serial.println("✅ WiFi ready, starting MQTT task");
    }
    
    TempHumid th;
    int water;
    unsigned long lastPublish = 0;
    static bool errorLogged = false;
    
    for (;;) {
        // ✅ Changed: Use 0 timeout instead of portMAX_DELAY to avoid blocking
        bool hasTempHumid = (TempHumidQueue != NULL && xQueuePeek(TempHumidQueue, &th, 0) == pdTRUE);
        bool hasWater = (waterValueQueue != NULL && xQueuePeek(waterValueQueue, &water, 0) == pdTRUE);

        Serial.println(hasTempHumid);
        Serial.println(hasWater);
        
        if (hasTempHumid && hasWater) {
#ifdef DEBUG
            Serial.println(coreiot_server);
            Serial.println(coreiot_port);
            Serial.println(coreiot_client_id);
            Serial.println(coreiot_username);
            Serial.println(WiFi.isConnected());
#endif
            if (WiFi.isConnected() &&
                coreiot_server != "" &&
                coreiot_port > 0 &&
                coreiot_client_id != "" &&
                coreiot_username != "") 
            { 
#ifdef DEBUG
                Serial.println("BOOTING COREIOT TASK");
#endif

                coreiot_loop(); 
                errorLogged = false;
                
                unsigned long now = millis();
                if (now - lastPublish >= 5000) {

#ifdef DEBUG
                Serial.println("PUBLISHING");
#endif

                    lastPublish = now;
                    
                    String json;
                    json = "{\"temperature\":" + String(th.temperature, 1) + 
                           ",\"humidity\":" + String(th.humidity, 1) + 
                           ",\"rain\":" + String((water*100)/4095) + 
                           ",\"status\":\"sensor_active\"}";
                    
                    Serial.println("\n✅ Publishing REAL sensor data:");
                    Serial.println("   Temperature: " + String(th.temperature) + "°C");
                    Serial.println("   Humidity   : " + String(th.humidity, 1) + "%");
                    Serial.println("   Rain (ADC%): " + String((water*100)/4095) + "%");
                    Serial.println("   JSON: " + json);
                    
                    publishData(json);
                }
            } 
            else 
            {
                if (!errorLogged) {
                    if (coreiot_server == "" || 
                        coreiot_port == 0 || 
                        coreiot_client_id == "" || 
                        coreiot_username == "") 
                    {
                        Serial.println("⚠️ CoreIOT config missing, please fill in Settings");
                    } 
                    else 
                    {
                        Serial.println("⚠️ WiFi disconnected, waiting to reconnect...");
                    }
                    errorLogged = true;
                }
            }
        } else {
            // ✅ Added: Handle case when sensor data not yet available
            if (!errorLogged) {
                Serial.println("⏳ Waiting for sensor data...");
                errorLogged = true;
            }
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
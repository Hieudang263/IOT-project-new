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

    unsigned long lastPublish = 0;
    bool testMode = true;            // Send test data if no sensors yet
    static bool errorLogged = false; // Log once until conditions change

    for (;;) {
        if (WiFi.isConnected() &&
            coreiot_server != "" &&
            coreiot_port > 0 &&
            coreiot_client_id != "" &&
            coreiot_username != "") 
        { 
            coreiot_loop(); 
            errorLogged = false;

            unsigned long now = millis();
            if (now - lastPublish >= 10000) {
                lastPublish = now;

                bool hasSensor = (!isnan(glob_temperature) &&
                  !isnan(glob_humidity) &&
                  !isnan(glob_rain) &&
                  glob_temperature != -1 &&
                  glob_humidity != -1 &&
                  glob_temperature != 0 &&  // <--- Thêm dòng này
                  glob_humidity != 0);      // <--- Thêm dòng này

                String json;
                if (hasSensor) {
                    json = "{\"temperature\":" + String(glob_temperature, 1) + 
                           ",\"humidity\":" + String(glob_humidity, 1) + 
                           ",\"rain\":" + String(glob_rain, 1) + 
                           ",\"status\":\"sensor_active\"}";
                    
                    Serial.println("\n✅ Publishing REAL sensor data:");
                    Serial.println("   Temperature: " + String(glob_temperature, 1) + "°C");
                    Serial.println("   Humidity   : " + String(glob_humidity, 1) + "%");
                    Serial.println("   Rain (ADC%): " + String(glob_rain, 1) + "%");
                    
                    testMode = false;
                } 
                else {
                    if (testMode) {
                        json = "{\"message\":\"hello this is test data\",\"status\":\"test_mode\",\"timestamp\":" + String(millis()) + "}";
                        Serial.println("\n🧪 Publishing TEST data (no sensor detected)");
                    } else {
                        json = "{\"status\":\"sensor_lost\",\"temperature\":0,\"humidity\":0,\"rain\":0}";
                        Serial.println("\n⚠️ Publishing SENSOR LOST warning");
                    }
                }

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
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

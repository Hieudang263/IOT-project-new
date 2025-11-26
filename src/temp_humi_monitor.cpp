#include "temp_humi_monitor.h"
#include "coreiot.h"
#include <ArduinoJson.h>

DHT20 dht20;
LiquidCrystal_I2C lcd(33, 16, 2);

#define RAIN_ADC_PIN 1      // ADC pin for rain sensor (change to your wiring)
#define ADC_MAX_VALUE 4095  // 12-bit ADC full scale on ESP32

void temp_humi_monitor(void *pvParameters) {
    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    while (1) {
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();
        int rainRaw = analogRead(RAIN_ADC_PIN);
        float rainPercent = constrain((rainRaw / ADC_MAX_VALUE) * 100.0f, 0.0f, 100.0f);

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity = -1;
        }

        glob_temperature = temperature;
        glob_humidity = humidity;
        glob_rain = rainPercent;

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("C");
        Serial.print("Rain (ADC%): ");
        Serial.print(rainPercent);
        Serial.println("%");

        StaticJsonDocument<128> doc;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["rain"] = rainPercent;

        String jsonData;
        serializeJson(doc, jsonData);
        publishData(jsonData);

        vTaskDelay(5000);
    }
}

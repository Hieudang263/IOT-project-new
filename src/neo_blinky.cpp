#include "neo_blinky.h"

void neo_blinky(void *pvParameters) {
    static Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    TempHumid receiver;

    while (1) {
        // Wait for queue data (non-blocking for too long)
        if (xQueuePeek(TempHumidQueue, &receiver, 100)) {

            strip.clear();

            if (receiver.humidity > 90) {
                strip.setPixelColor(0, strip.Color(128, 0, 0));  // Dark Red
                Serial.println("TOO HUMID");
            }
            else if (receiver.humidity > 80) {
                strip.setPixelColor(0, strip.Color(220, 20, 60)); // Crimson
            }
            else if (receiver.humidity > 70) {
                strip.setPixelColor(0, strip.Color(255, 255, 0)); // Yellow
            }
            else if (receiver.humidity > 60) {
                strip.setPixelColor(0, strip.Color(0, 100, 0));   // Dark Green
            }
            else if (receiver.humidity > 50) {
                strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan
            }
            else {
                strip.setPixelColor(0, strip.Color(0, 0, 255));   // Blue (default low humidity)
            }

            strip.show();
        }

        vTaskDelay(500);  // Update every 0.5 sec
    }
}
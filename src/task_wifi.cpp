#include "task_wifi.h"
#include "esp_wpa2.h"  

bool startSTA(bool stopAP)
{
    if (WIFI_SSID.isEmpty()) {
        Serial.println("⚠️ WIFI_SSID empty, cannot connect");
        return false;
    }

    Serial.println("\n======== CONNECTING WIFI ========");
    Serial.println("SSID: " + WIFI_SSID);
    Serial.println("Pass: " + String(WIFI_PASS.length()) + " chars");
    
    // ✅ CHECK NẾU CÓ USERNAME → WPA2-ENTERPRISE
    if (!WIFI_USERNAME.isEmpty()) {
        Serial.println("🔐 WPA2-Enterprise mode");
        Serial.println("Username: " + WIFI_USERNAME);
        
        // ✅ VALIDATE PASSWORD
        if (WIFI_PASS.isEmpty()) {
            Serial.println("❌ ERROR: Password is empty!");
            Serial.println("💡 WPA2-Enterprise REQUIRES both username AND password");
            return false;
        }
        
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        delay(100);
        
        esp_wifi_sta_wpa2_ent_enable();
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME.c_str(), WIFI_USERNAME.length());
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME.c_str(), WIFI_USERNAME.length());
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASS.c_str(), WIFI_PASS.length());
        
        WiFi.begin(WIFI_SSID.c_str());
    } else {
        Serial.println("🔓 WPA2-PSK mode (normal WiFi)");
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 200) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (timeout % 10 == 0) Serial.print(".");
        timeout++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi Connected!");
        Serial.println("IP: " + WiFi.localIP().toString());
        
        if (stopAP) {
            Serial.println("ℹ️ Turning off AP...");
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            Serial.println("✅ AP stopped, STA only");
        }
        
        if (xBinarySemaphoreInternet != NULL) {
            xSemaphoreGive(xBinarySemaphoreInternet);
        }
        return true;
    }
    
    // ✅ CHI TIẾT LÝ DO THẤT BẠI
    wl_status_t status = WiFi.status();
    Serial.println("❌ WiFi connection FAILED");
    Serial.println("=================================");
    
    switch(status) {
        case WL_NO_SSID_AVAIL:
            Serial.println("📍 Lỗi: SSID không tồn tại");
            Serial.println("💡 Kiểm tra:");
            Serial.println("   - Tên WiFi đúng chưa?");
            Serial.println("   - Router có bật không?");
            Serial.println("   - ESP32 có ở gần router không?");
            break;
            
        case WL_CONNECT_FAILED:
            if (!WIFI_USERNAME.isEmpty()) {
                Serial.println("🔐 Lỗi: Sai Username hoặc Password (WPA2-Enterprise)");
                Serial.println("💡 Kiểm tra:");
                Serial.println("   - Username: " + WIFI_USERNAME);
                Serial.println("   - Password có đúng không?");
                Serial.println("   - Tài khoản còn active không?");
            } else {
                Serial.println("🔑 Lỗi: Sai mật khẩu WiFi");
                Serial.println("💡 Kiểm tra mật khẩu trong /info.dat");
            }
            break;
            
        case WL_DISCONNECTED:
            Serial.println("⏱️ Lỗi: Timeout hoặc từ chối kết nối");
            Serial.println("💡 Nguyên nhân có thể:");
            Serial.println("   - Router quá tải");
            Serial.println("   - Giới hạn số thiết bị");
            Serial.println("   - MAC address bị chặn");
            break;
            
        default:
            Serial.printf("❓ Lỗi không xác định (Status: %d)\n", status);
            Serial.println("💡 Thử:");
            Serial.println("   - Restart ESP32");
            Serial.println("   - Kiểm tra Serial Monitor");
    }
    
    Serial.println("=================================\n");
    return false;
}

bool Wifi_reconnect()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    if (WIFI_SSID.isEmpty()) {
        return false;
    }
    
    Serial.println("📡 WiFi reconnecting...");
    
    WiFi.disconnect(false);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    // ✅ HỖ TRỢ WPA2-ENTERPRISE KHI RECONNECT
    if (!WIFI_USERNAME.isEmpty()) {
        Serial.println("🔐 Reconnecting WPA2-Enterprise...");
        
        WiFi.mode(WIFI_STA);
        delay(100);
        
        esp_wifi_sta_wpa2_ent_enable();
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME.c_str(), WIFI_USERNAME.length());
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME.c_str(), WIFI_USERNAME.length());
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASS.c_str(), WIFI_PASS.length());
        
        WiFi.begin(WIFI_SSID.c_str());
    } else {
        Serial.println("🔓 Reconnecting WPA2-PSK...");
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 150) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi reconnected: " + WiFi.localIP().toString());
        
        if (xBinarySemaphoreInternet != NULL) {
            xSemaphoreGive(xBinarySemaphoreInternet);
        }
        return true;
    }
    
    Serial.println("❌ WiFi reconnect failed");
    return false;
}

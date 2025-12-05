#ifndef TASK_WIFI_H
#define TASK_WIFI_H

#include <WiFi.h>
#include <task_check_info.h>
#include <task_webserver.h>

// Keep if used across files
bool Wifi_reconnect();

// Return true if STA connects; stopAP=true will turn off softAP on success
bool startSTA(bool stopAP = true);

#endif

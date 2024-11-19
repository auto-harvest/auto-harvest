#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFiEspAT.h>
#include "services/disk-manager/diskManager.service.h"

class WiFiService
{
public:
    WiFiService(DiskManagerService *diskManager);
    void begin();

    void scanNetworks();
    void turnToAccessPointMode(const char *ssid, const char *password);
    void turnToNormalMode();
    void connectToWiFi(const char *ssid, const char *password);

    DiskManagerService *diskManager;
};

#endif // WIFI_SERVICE_H
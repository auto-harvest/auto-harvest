#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFiEspAT.h>

class WiFiService
{
public:
    WiFiService();
    String begin();

    void scanNetworks();
    void turnToAccessPointMode(const char *ssid, const char *password);
    void turnToNormalMode();
    void connectToWiFi(const char *ssid, const char *password);
    String getMacAddress();
    String mode;
};

#endif // WIFI_SERVICE_H
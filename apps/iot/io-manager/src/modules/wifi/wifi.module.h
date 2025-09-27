#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include "module.abstract.class.h"
#include "WiFi.h"
class WiFiModule : public AbstractModule
{
public:
    WiFiModule();
    ~WiFiModule();

    void initialize() override;
    const char *getType() override;
    const char *getName() override;
    const char *getStatus() override;
    void setPower(bool power) override;

    void scanNetworks();
    void turnToAccessPointMode(const char *ssid, const char *password);
    void turnToNormalMode();
    void connectToWiFi(const char *ssid, const char *password);

private:
    String status;
};

#endif // WIFI_MODULE_H
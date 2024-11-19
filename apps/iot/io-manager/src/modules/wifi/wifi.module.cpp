#include "wifi.module.h"

WiFiModule::WiFiModule() : status("Initializing...")
{
}

WiFiModule::~WiFiModule()
{
}

void WiFiModule::initialize()
{
 
    status = "Initialized";
}

const char *WiFiModule::getType()
{
    return "WiFi";
}

const char *WiFiModule::getName()
{
    return "WiFi Module";
}

const char *WiFiModule::getStatus()
{
    return status.c_str();
}

void WiFiModule::setPower(bool power)
{
    if (power)
    {
        // Turn on the WiFi module
        // WiFi.begin();
        status = "WiFi Module is ON";
    }
    else
    {
        // Turn off the WiFi module
        WiFi.disconnect();
        status = "WiFi Module is OFF";
    }
}

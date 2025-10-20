#include "moduleManager.service.h"
#include <Arduino.h>

ModuleManager *ModuleManager::instance = nullptr;

ModuleManager::ModuleManager()
{

    ModuleManager::instance = this;
}

ModuleManager::~ModuleManager()
{
    ModuleManager::instance = nullptr;
}

void ModuleManager::initializeModules(DataCollector *dataCollector, WiFiService *wifiService, ActiveMQClientService *mqttService)
{
    // Initialize the LCD module with WiFi service and MQTT service
    lcd = new LCDModule(dataCollector, wifiService, mqttService);
    relays.push_back(new SingleRelay(24));
    relays.push_back(new SingleRelay(25));
    relays.push_back(new SingleRelay(26));
    relays.push_back(new SingleRelay(27));
    waterPump = new PumpModule(relays.at(0));
    airPump = new AirPumpModule(relays.at(1));
    // Initialize each module
    lcd->initialize();
    waterPump->initialize();
    airPump->initialize();
}

void ModuleManager::setMqttService(ActiveMQClientService *mqttService)
{
    // This can be called later to update the MQTT service reference if needed
    // Currently not used since we pass it during initialization
}
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

void ModuleManager::initializeModules(DataCollector *dataCollector)
{
    // Initialize the LCD module
    lcd = new LCDModule(dataCollector);
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
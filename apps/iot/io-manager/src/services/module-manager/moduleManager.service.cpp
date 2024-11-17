#include "moduleManager.service.h"
#include <Arduino.h>

ModuleManager::ModuleManager()
{
    // Initialize modules here
}

ModuleManager::~ModuleManager()
{
}

void ModuleManager::initializeModules(DataCollector *dataCollector)
{
    // Initialize the LCD module
    lcdModule = new LCDModule(dataCollector);
    relays.push_back(new SingleRelay(24));
    relays.push_back(new SingleRelay(25));
    relays.push_back(new SingleRelay(26));
    relays.push_back(new SingleRelay(27));
    waterPump = new PumpModule(relays.at(0));
    // Initialize each module

    waterPump->initialize();
}
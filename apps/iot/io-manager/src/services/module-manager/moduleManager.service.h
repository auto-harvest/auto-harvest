#ifndef MODULE_MANAGER_SERVICE_H
#define MODULE_MANAGER_SERVICE_H

#include <memory>
#include <vector>

#include "module.abstract.class.h"
#include "modules/pump/pump.module.h"
#include "modules/relay/single-relay.module.h"
#include "modules/lcd/lcd.module.h"
#include "modules/wifi/wifi.module.h"
#include "services/data-collector/dataCollector.service.h"
class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    void initializeModules(DataCollector *dataCollector);

    // Public references to all modules
    PumpModule *waterPump;
    std::vector<SingleRelay *> relays;
    LCDModule *lcd;

private:
    // Add other module references here as needed
};

#endif // MODULE_MANAGER_SERVICE_H
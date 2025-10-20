#ifndef MODULE_MANAGER_SERVICE_H
#define MODULE_MANAGER_SERVICE_H

#include <memory>
#include <vector>

#include "module.abstract.class.h"
#include "modules/pump/pump.module.h"
#include "modules/relay/single-relay.module.h"
#include "modules/lcd/lcd.module.h"
#include "modules/wifi/wifi.module.h"
#include "modules/air-pump/air-pump.module.h"
#include "services/data-collector/dataCollector.service.h"
#include "services/wifi-manager/wifiManager.service.h"
#include "services/activeMQ-client/activeMQ-client.service.h"
class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    void initializeModules(DataCollector *dataCollector, WiFiService *wifiService, ActiveMQClientService *mqttService = nullptr);
    void setMqttService(ActiveMQClientService *mqttService);

    // Public references to all modules
    PumpModule *waterPump;
    std::vector<SingleRelay *> relays;
    LCDModule *lcd;
    AirPumpModule *airPump;

private:
    static ModuleManager *instance;
    // Add other module references here as needed
};

#endif // MODULE_MANAGER_SERVICE_H
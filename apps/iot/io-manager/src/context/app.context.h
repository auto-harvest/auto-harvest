
#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "services/data-collector/dataCollector.service.h"
#include "services/module-manager/moduleManager.service.h"
#include "services/disk-manager/diskManager.service.h"
#include "services/wifi-manager/wifiManager.service.h"
#include "services/webserver/webserver.service.h"
#include "services/activeMQ-client/activeMQ-client.service.h"
#include "utility/timer.util.h"
#include "abstract/singleton.h"
#include "abstract/uniquePointer.h" // Include the custom UniquePtr implementation

class AppContext : public Singleton<AppContext>
{
    friend class Singleton<AppContext>; // Allow Singleton to access the private constructor

public:
    ActiveMQClientService *activeMQService;
    DataCollector *dataCollector;
    ModuleManager *moduleManager;
    DiskManagerService *diskManager;
    WiFiService *wifiService;
    WebServerService *webServerService;
    Timer sensorPollTimer;
    Timer lcdUpdateTimer;
    Timer dataSendTimer;
    Timer eventHandleTimer;
    String ssid, password, brokerAddress, clientId;
    void initialize();
    void handleEvents();
    void loop();
private:
    AppContext();
    ~AppContext();
    AppContext(const AppContext &) = delete;
    AppContext &operator=(const AppContext &) = delete;
};

#endif // APP_CONTEXT_H
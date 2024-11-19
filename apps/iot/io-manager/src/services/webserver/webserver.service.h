#ifndef WEBSERVER_SERVICE_H
#define WEBSERVER_SERVICE_H

#include <WiFiEspAT.h>
#include "services/disk-manager/diskManager.service.h"
#include "services/wifi-manager/wifiManager.service.h"
class WebServerService
{
public:
    WebServerService(DiskManagerService *diskManager);
    void begin();
    void handleClient();

private:
    WiFiServer server;
    DiskManagerService *diskManager;
    WiFiService *wifiService;
    void handleRoot(WiFiClient &client);
    void handleSaveCredentials(WiFiClient &client, String &body);
    String readRequestBody(WiFiClient &client, int contentLength);
};

#endif

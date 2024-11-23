#include "app.context.h"
// Replace raw pointers with UniquePtr
UniquePtr<ActiveMQClientService> activeMQService;
UniquePtr<DataCollector> dataCollector;
UniquePtr<ModuleManager> moduleManager;
UniquePtr<DiskManagerService> diskManager;
UniquePtr<WiFiService> wifiService;
UniquePtr<WebServerService> webServerService;

AppContext::AppContext()
    : activeMQService(new ActiveMQClientService()),
      dataCollector(new DataCollector()),
      moduleManager(new ModuleManager()),
      diskManager(new DiskManagerService()),
      wifiService(new WiFiService()),
      webServerService(new WebServerService(*diskManager, *wifiService)),
      sensorPollTimer(2000),
      lcdUpdateTimer(1000),
      dataSendTimer(10000),
      eventHandleTimer(1000) {}

// No need to manually delete resources in the destructor
AppContext::~AppContext() = default;

void AppContext::initialize()
{
    Serial.begin(115200);
    // initialize
    Wire.begin();
    dataCollector->initializeSensors();

    moduleManager->initializeModules(dataCollector);

    diskManager->initialize();

    dataCollector->collectData();
    dataCollector->printData(dataCollector->currentData);

    clientId = wifiService->begin();
    Serial.println("Client ID: " + clientId);
    delay(1000);
    // diskManager.remove();
    ssid = diskManager->read("ssid");
    password = diskManager->read("password");
    brokerAddress = diskManager->read("brokerAddress");
    if (ssid.length() > 0 && password.length() > 0)
    {
        Serial.println("Credentials found, connecting to WiFi...");
        wifiService->connectToWiFi(ssid.c_str(), password.c_str());
        activeMQService->initialize(brokerAddress.c_str(), 3011, clientId.c_str());
        activeMQService->subscribe("pump-on");
        activeMQService->subscribe("pump-off");
        activeMQService->subscribe("air-pump-on");
        activeMQService->subscribe("air-pump-off");
        activeMQService->subscribe("scan-networks");
        activeMQService->subscribe("connect-to-wifi");
        activeMQService->subscribe("turn-to-ap");
        activeMQService->subscribe("set-sensor-poll-interval");
        activeMQService->subscribe("close-lcd");
        activeMQService->subscribe("open-lcd");
    }
    else
    {
        Serial.println("No credentials found, turning to Access Point mode...");
        wifiService->turnToAccessPointMode("ESP8266", "12345678");
        webServerService->begin();
    }
}

void AppContext::loop()
{
    if (wifiService->mode == "ap")
    {
        Serial.println("In AP mode");
        if (webServerService->getState() == "stopped")
        {
            webServerService->begin();
        }
        webServerService->handleClient();
        return;
    }
    if (!activeMQService->isConnected())
    {
        activeMQService->initialize(brokerAddress.c_str(), 3011, clientId.c_str());
    }
    activeMQService->loop();
    while (activeMQService->hasMessage())
    {
        auto message = activeMQService->getNextMessage();
        JsonDocument response;
        // split topic with "/" separator
        String topic = message.topic;
        if (topic == "pump-on")
        {
            moduleManager->waterPump->setPower(true);
            response["pump-status"] = "on";
            activeMQService->publish("pump-status", response);
        }
        else if (topic == "pump-off")
        {
            moduleManager->waterPump->setPower(false);
            activeMQService->publish("pump-status", response);
        }
        else if (topic == "air-pump-on")
        {
            moduleManager->airPump->setPower(true);
            activeMQService->publish("air-pump-status", response);
        }
        else if (topic == "air-pump-off")
        {
            moduleManager->airPump->setPower(false);
            activeMQService->publish("air-pump-status", response);
        }
        else if (topic == "scan-networks")
        {
            wifiService->scanNetworks();
            activeMQService->publish("networks", response);
        }
        else if (topic == "connect-to-wifi")
        {
            wifiService->connectToWiFi(message.message.c_str(), "12345678");
            activeMQService->publish("wifi-status", response);
        }
        else if (topic == "turn-to-ap")
        {
            wifiService->turnToAccessPointMode("ESP8266", "12345678");

            // activeMQService->publish("wifi-status", response);
        }
        else if (topic == "set-sensor-poll-interval")
        {
            sensorPollTimer = Timer(message.message.toInt());
            activeMQService->publish("sensor-poll-interval", response);
        }
        else if (topic == "close-lcd")
        {
            moduleManager->lcd->setPower(false);
            activeMQService->publish("lcd-status", response);
        }
        else if (topic == "open-lcd")
        {
            moduleManager->lcd->setPower(true);
            activeMQService->publish("lcd-status", response);
        }

        Serial.println("Message received: " + message.topic + " - " + message.message);
    }

    if (sensorPollTimer.canRun())
    {

        // moduleManager->lcd->update();
    }

    if (lcdUpdateTimer.canRun())
    {
        moduleManager->lcd->update("Sensor Data");
        moduleManager->lcd->displaySensorData();
    }

    if (sensorPollTimer.canRun())
    {

        auto data = dataCollector->collectData();
        if (data.size() > 0)
        {
            // auto json = dataToJson(data);
            JsonDocument doc;
            for (auto &entry : data)
            {
                doc[entry.first] = entry.second;
            }
            activeMQService->publish("sensor-data", doc);
        }
    }
}
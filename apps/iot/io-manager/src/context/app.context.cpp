#include "app.context.h"
#include "config.h"
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
      sensorPollTimer(1000),
      lcdUpdateTimer(2000),
      dataSendTimer(2000),
      eventHandleTimer(1000) {}

// No need to manually delete resources in the destructor
AppContext::~AppContext() = default;

void AppContext::initialize()
{

    dataCollector->initializeSensors();

    moduleManager->initializeModules(dataCollector, wifiService, activeMQService);

    diskManager->initialize();

    dataCollector->collectData();
    dataCollector->printData(dataCollector->currentData);

    clientId = wifiService->begin();
    Serial.println("Client ID: " + clientId);
    // diskManager->remove("ssid");
    // diskManager->remove("password");
    ssid = "TI_EIXAME_TI_XASAME"; // diskManager->read("ssid");
    password = "denkserw";        // diskManager->read("password");

    // Build broker address from config.h settings
    brokerAddress = String("mqtt://") + MQTT_BROKER_HOST + ":" + String(MQTT_BROKER_PORT);
    if (ssid.length() > 0 && password.length() > 0)
    {
        Serial.println("Credentials found, connecting to WiFi...");
        wifiService->connectToWiFi(ssid.c_str(), password.c_str());
        activeMQService->initialize(MQTT_BROKER_HOST, MQTT_BROKER_PORT, clientId.c_str());
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
        activeMQService->subscribe("time-sync");
    }
    else
    {
        Serial.println("No credentials found, turning to Access Point mode...");
        wifiService->turnToAccessPointMode(AP_SSID, AP_PASSWORD);
        webServerService->begin();
    }
}

void AppContext::loop()
{
    if (wifiService->mode == "ap")
    {
        if (webServerService->getState() == "stopped")
        {
            webServerService->begin();
        }
        webServerService->handleClient();
        return;
    }
    if (!activeMQService->isConnected())
    {
        activeMQService->initialize(MQTT_BROKER_HOST, MQTT_BROKER_PORT, clientId.c_str());
    }

    activeMQService->loop();
    while (activeMQService->hasMessage())
    {

        auto message = activeMQService->getNextMessage();
        Serial.println("Message received: " + message.topic + " - " + message.message);
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
        else if (topic == "time-sync")
        {
            // Parse JSON: {"time": "14:30:45", "date": "20/10/2025"}
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, message.message);

            if (!error && doc.containsKey("time") && doc.containsKey("date"))
            {
                String time = doc["time"].as<String>();
                String date = doc["date"].as<String>();
                moduleManager->lcd->setTime(time, date);
                Serial.println("Time synced: " + time + " " + date);
            }
        }

        Serial.println("Message received: " + message.topic + " - " + message.message);
    }

    if (sensorPollTimer.canRun())
    {
        auto data = dataCollector->collectData();
        // moduleManager->lcd->update();
    }

    if (lcdUpdateTimer.canRun())
    {
        // Update carousel to cycle through pages
        moduleManager->lcd->updateCarousel();
    }

    if (dataSendTimer.canRun())
    {

        if (dataCollector->currentData.size() > 0)
        {

            JsonDocument jsonDoc;
            jsonDoc["client-id"] = clientId;
            for (const auto &entry : dataCollector->currentData)
            {
                jsonDoc[entry.first] = entry.second;
            }
            serializeJson(jsonDoc, Serial);
            // deserializeJson(jsonDoc, "{\"temperature\": 25.0, \"humidity\": 50.0, \"ph\": 7.0, \"tds\": 100.0}");
            activeMQService->publish("sensor-data", jsonDoc);
            Serial.println(jsonDoc.overflowed());
        }
    }
}
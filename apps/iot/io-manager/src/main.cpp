#include <Arduino.h>
#include "sensors/water-level/waterLevel.sensor.h"
#include <Wire.h>
#include "services/data-collector/dataCollector.service.h"
#include "services/module-manager/moduleManager.service.h"
#include "services/disk-manager/diskManager.service.h"
#include "services/wifi-manager/wifiManager.service.h" // Include the header file for WiFiService
#include "services/webserver/webserver.service.h"      // Include the header file for WebServerService
#include <EEPROM.h>
void logEEPROMContent();

String previous = "";
int i = 0;

auto dataCollector = new DataCollector();
auto moduleManager = new ModuleManager();
auto diskManager = new DiskManagerService();
auto wifiService = new WiFiService(diskManager);
auto webServerService = new WebServerService(diskManager);

void setup()
{
    // Initialize serial communication for debugging
    Serial.begin(115200);
    // initialize
    Wire.begin();
    Serial.println("Scanning for I2C devices...");

    Serial.println("Scanning complete.");

    // Initialize the relays
    dataCollector->initializeSensors();
    moduleManager->initializeModules(dataCollector);
    wifiService->begin();
    diskManager->initialize();

    diskManager->purge();
    String ssid = diskManager->read("ssid");
    String password = diskManager->read("password");
    Serial.println("SSID: " + ssid);
    if (ssid.length() > 0 && password.length() > 0)
    {
        Serial.println("Credentials found, connecting to WiFi...");
        wifiService->connectToWiFi(ssid.c_str(), password.c_str());
    }
    else
    {
        Serial.println("No credentials found, turning to Access Point mode...");
        wifiService->turnToAccessPointMode("ESP8266", "12345678");
        webServerService->begin();
    }
    logEEPROMContent();
}

int r, g, b, t = 0;

void loop()
{
    // Check the state of the water level sensor
    webServerService->handleClient();
    // Serial.println("Water level is: " + waterLevelIs());
    i += 1;
    // auto data = dataCollector->collectData();
    String message = "Ticks... " + String(i);
    moduleManager->lcd->update(message.c_str());
    // moduleManager->lcd->displaySensorData();

    delay(100);
}
void logEEPROMContent()
{
    String eepromContent = "";

    for (int i = 0; i < 4096; i++)
    {
        char ch = EEPROM.read(i);
        if (ch == 0)
        { // Stop at the null terminator
            break;
        }
        eepromContent += ch;
    }

    Serial.println("EEPROM Content as String:");
    Serial.println(eepromContent);
}

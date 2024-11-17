#include <Arduino.h>
#include "sensors/water-level/waterLevel.sensor.h"
#include <Wire.h>
#include "modules/relay/relay.module.h"
#include "services/data-collector/dataCollector.service.h"
#include "services/module-manager/moduleManager.service.h"

void show(char *str);
String previous = "";
int i = 0;
auto dataCollector = new DataCollector();
auto moduleManager = new ModuleManager();
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
}
int r, g, b, t = 0;

void loop()
{
    // Check the state of the water level sensor

    // Serial.println("Water level is: " + waterLevelIs());
    Serial.println("-----------------------------------");
    i += 1;
    auto data = dataCollector->collectData();
    moduleManager->lcdModule->update("HIIII");
    delay(1000); // Wait for 1 second before checking again

    delay(1000);
}

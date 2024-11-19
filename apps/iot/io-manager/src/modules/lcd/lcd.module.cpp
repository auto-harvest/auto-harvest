#include "lcd.module.h"

// Constructor
LCDModule::LCDModule(DataCollector *dataCollector) : dataCollector(dataCollector), status("Initializing...")
{
    screen = new Waveshare_LCD1602(16, 2);
}

// Destructor
LCDModule::~LCDModule()
{
    // Destructor implementation (if needed)
}

// Initialize the LCD module
void LCDModule::initialize()
{
    Serial.println("Initializing LCD Module...");
    screen->init();

    screen->setCursor(0, 0);
    screen->send_string("AutoHarvest v0.111");
    screen->send_string(status.c_str());
    screen->blink();
}

// Update the LCD display
void LCDModule::update(const char *status)
{
    screen->setCursor(0, 0);
    // pad status with spaces so it reaches 16 characters
    String statusString = status;
    statusString.concat("                ");
    statusString = statusString.substring(0, 16);
    screen->send_string(statusString.c_str());
}

void LCDModule::displaySensorData()
{
    screen->setCursor(0, 1);
    auto data = dataCollector->currentData;
    String sensorData;
    String temperature = String(data["temperature"], 1) + "C " + String(data["humidity"], 0) + "%" + " " + String(data["tds"], 0) + "ppm " + String(data["water-temperature"], 1) + "C";

    screen->send_string(temperature.c_str());
}

// Get the type of the module
const char *LCDModule::getType()
{
    return "LCD";
}

// Get the name of the module
const char *LCDModule::getName()
{
    return "LCD Module";
}

// Get the status of the module
const char *LCDModule::getStatus()
{
    return status.c_str();
}

// Set the power state of the module
void LCDModule::setPower(bool power)
{
    // Set the power state of the module
    if (power)
    {
        status = "ON";
    }
    else
    {
        status = "OFF";
    }
}
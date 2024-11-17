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

    screen->init();
    screen->setCursor(0, 0);
    screen->send_string("AutoHarvest v0.1");
    screen->setCursor(0, 1);
    screen->send_string(status.c_str());
    screen->blink();
}

// Update the LCD display
void LCDModule::update(const char *status)
{
    screen->clear();
    screen->setCursor(0, 0);
    screen->send_string(status);

    auto data = dataCollector->collectData();
    String sensorData;

    screen->setCursor(0, 1);
    screen->send_string("Some data");
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
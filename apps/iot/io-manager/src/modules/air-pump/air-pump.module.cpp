#include "air-pump.module.h"
#include <Arduino.h>

AirPumpModule::AirPumpModule(SingleRelay *relay) : relay(relay)
{
}

AirPumpModule::~AirPumpModule()
{
}

void AirPumpModule::initialize()
{
    Serial.println("Initializing Air Pump Module...");

    relay->initialize();
}

const char *AirPumpModule::getType()
{
    return "Pump";
}

const char *AirPumpModule::getName()
{
    return "12VDC Air Pump";
}

const char *AirPumpModule::getStatus()
{
    return powerState ? "On" : "Off";
}

void AirPumpModule::setPower(bool power)
{
    // print state
    Serial.print("Setting pump power to: ");
    Serial.println(power ? "On" : "Off");
    powerState = power;
    relay->setPower(power);
}
#include "pump.module.h"
#include <Arduino.h>

PumpModule::PumpModule(SingleRelay *relay) : relay(relay)
{
}

PumpModule::~PumpModule()
{
}

void PumpModule::initialize()
{
    relay->initialize();
}

const char *PumpModule::getType()
{
    return "Pump";
}

const char *PumpModule::getName()
{
    return "12VDC Water Pump";
}

const char *PumpModule::getStatus()
{
    return powerState ? "On" : "Off";
}

void PumpModule::setPower(bool power)
{
    //print state
    Serial.print("Setting pump power to: ");
    Serial.println(power ? "On" : "Off");
    powerState = power;
    relay->setPower(power);
}
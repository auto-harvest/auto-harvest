#include "waterTemperature.sensor.h"

WaterTemperatureSensor::WaterTemperatureSensor(uint8_t pin) : pin(pin), oneWire(pin), sensors(&oneWire)
{
}

WaterTemperatureSensor::~WaterTemperatureSensor()
{
    // Destructor implementation (if needed)
}

void WaterTemperatureSensor::initialize()
{
    sensors.begin();
    sensors.setResolution(11);
}

std::map<std::string, double> WaterTemperatureSensor::readData()
{
    std::map<std::string, double> data;
    sensors.requestTemperatures();
    this->lastTemperature = sensors.getTempCByIndex(0);
    data["wt"] = lastTemperature;
    return data;
}

const char *WaterTemperatureSensor::getType()
{
    return "DS18B20";
}

const char *WaterTemperatureSensor::getSensorName()
{
    return "DS18B20";
}
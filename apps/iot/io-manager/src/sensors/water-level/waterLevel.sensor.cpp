#include "waterLevel.sensor.h"

WaterLevelSensor::WaterLevelSensor(uint8_t lowPin, uint8_t highPin) : lowPin(lowPin), highPin(highPin)
{
    this->lowPin = lowPin;
    this->highPin = highPin;
}

WaterLevelSensor::~WaterLevelSensor()
{
    // Destructor implementation (if needed)
}
void WaterLevelSensor::initialize()
{
    pinMode(this->lowPin, INPUT_PULLUP);
    pinMode(this->highPin, INPUT_PULLUP);
}

std::map<std::string, double> WaterLevelSensor::readData()
{
    std::map<std::string, double> data;
    data["water-level"] = waterLevelIs();
    return data;
}

const char *WaterLevelSensor::getType()
{
    return "WaterLevel";
}

const char *WaterLevelSensor::getSensorName()
{
    return "Water Level Sensor";
}

bool WaterLevelSensor::isWaterLevelLow()
{
    return digitalRead(lowPin) == LOW;
}

bool WaterLevelSensor::isWaterLevelAdequate()
{
    return digitalRead(lowPin) == HIGH && digitalRead(highPin) == LOW;
}

bool WaterLevelSensor::isWaterLevelHigh()
{
    return digitalRead(highPin) == HIGH;
}

bool WaterLevelSensor::isGravityReversed()
{
    return digitalRead(lowPin) == LOW && digitalRead(highPin) == HIGH;
}

double WaterLevelSensor::waterLevelIs()
{
    if (isWaterLevelLow())
    {
        return -1.0;
    }
    else if (isWaterLevelAdequate())
    {
        return 0.0;
    }
    else if (isWaterLevelHigh())
    {
        return 1.0;
    }
    else if (isGravityReversed())
    {
        return 101010.0;
    }
    return 101010.0;
}
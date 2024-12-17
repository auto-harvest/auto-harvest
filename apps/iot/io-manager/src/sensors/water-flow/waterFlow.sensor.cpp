#include "waterFlow.sensor.h"

WaterFlowSensor *WaterFlowSensor::instance = nullptr;

WaterFlowSensor::WaterFlowSensor(uint8_t pin)
    : pulses(0), lastPulseTime(0), flowRate(0), pin(pin)
{
    instance = this;
}

WaterFlowSensor::~WaterFlowSensor()
{
    instance = nullptr;
    detachInterrupt(digitalPinToInterrupt(pin));
}

void WaterFlowSensor::initialize()
{
    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), flowSensorISR, RISING);
    Serial.println("Flow sensor initialized");
}

std::map<std::string, double> WaterFlowSensor::readData()
{
    noInterrupts(); // Ensure consistent data access
    float currentFlowRate = flowRate;
    uint16_t currentPulses = pulses;
    unsigned long timeSinceLastPulse = millis() - lastPulseTime;
    interrupts();

    // Reset flow rate if no pulse detected in the last 1 second
    if (timeSinceLastPulse > 1000)
    {
        currentFlowRate = 0;
    }

    // Calculate total liters
    float liters = currentPulses / (7.5 * 60.0);

    std::map<std::string, double> data;
    data["flow-rate-hz"] = currentFlowRate;
    data["pulses"] = currentPulses;
    data["flow-rate-liters"] = liters;
    data["liters-per-minute"] = currentFlowRate * 60.0;
    return data;
}

const char *WaterFlowSensor::getType()
{
    return "Flow Sensor";
}

const char *WaterFlowSensor::getSensorName()
{
    return "YF-S201";
}

void WaterFlowSensor::flowSensorISR()
{
    unsigned long currentTime = millis();

    // Calculate time between pulses
    unsigned long elapsedTime = currentTime - instance->lastPulseTime;

    if (elapsedTime > 0)
    {
        instance->flowRate = 1000.0 / elapsedTime; // Hz (pulses per second)
    }

    instance->pulses++;
    instance->lastPulseTime = currentTime;
}

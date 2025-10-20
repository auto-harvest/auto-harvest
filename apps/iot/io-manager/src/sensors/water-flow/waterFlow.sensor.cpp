#include "waterFlow.sensor.h"

WaterFlowSensor *WaterFlowSensor::instance = nullptr;

WaterFlowSensor::WaterFlowSensor(uint8_t pin)
    : pulses(0), lastPulseTime(0), lastInterruptTime(0), flowRate(0), pin(pin)
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
    attachInterrupt(digitalPinToInterrupt(pin), flowISR, FALLING);
    Serial.println("Flow sensor initialized");
}

std::map<std::string, double> WaterFlowSensor::readData()
{
    std::map<std::string, double> data;

    float currentFlowRate = flowRate;
    uint16_t currentPulses = pulses;
    unsigned long timeSinceLastPulse = millis() - lastPulseTime;

    // Reset flow rate if no pulse detected in the last 1 second
    if (timeSinceLastPulse > 1000)
    {
        currentFlowRate = 0;
    }

    // Calculate total liters
    float liters = currentPulses / (7.5 * 60.0);

    // data["flow-rate-hz"] = currentFlowRate;
    // data["pulses"] = currentPulses;
    // data["flow-rate-liters"] = liters;
    data["lpm"] = currentFlowRate; // flowRate is already in L/min from ISR
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

void WaterFlowSensor::flowISR()
{
    // NULL check to prevent crashes
    if (instance == nullptr)
    {
        return;
    }

    unsigned long currentTime = millis();

    // Debounce: ignore pulses within 10ms of each other
    if (currentTime - instance->lastInterruptTime < 10)
    {
        return;
    }

    instance->lastInterruptTime = currentTime;
    instance->pulses++;

    // Calculate flow rate based on pulse frequency
    // Time between pulses in seconds
    float timeDiff = (currentTime - instance->lastPulseTime) / 1000.0;

    if (timeDiff > 0)
    {
        // YF-S201: Flow rate (L/min) = Frequency (Hz) / 7.5
        // Optimized: 1/timeDiff/7.5 = 1/(timeDiff*7.5) = 0.1333.../timeDiff
        instance->flowRate = 0.13333333f / timeDiff;
    }

    instance->lastPulseTime = currentTime;
}

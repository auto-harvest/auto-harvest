#ifndef WATERFLOW_SENSOR_H
#define WATERFLOW_SENSOR_H

#include <map>
#include <string>
#include <Arduino.h>
#include "sensor.abstract.class.h"
class WaterFlowSensor : public AbstractSensor
{
public:
    explicit WaterFlowSensor(uint8_t pin);
    ~WaterFlowSensor();

    void initialize();
    std::map<std::string, double> readData();
    const char *getType();
    const char *getSensorName();

    uint8_t pin;

    // Shared variables between ISR and main code
    volatile uint16_t pulses;
    volatile uint32_t lastFlowRateTimer;
    volatile float flowRate;
    volatile uint8_t lastFlowPinState;
    volatile long lastPulseTime;
    static WaterFlowSensor *instance; // For static ISR access
    void useInterrupt(boolean v);     // Interrupt handler
};

#endif // WATERFLOW_SENSOR_H

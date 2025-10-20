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
    volatile unsigned long lastPulseTime;
    volatile unsigned long lastInterruptTime; // For debouncing
    volatile float flowRate;
    static WaterFlowSensor *instance; // For static ISR access
    static void flowISR(); // Pin change ISR (simple, fast, no millis())
};

#endif // WATERFLOW_SENSOR_H

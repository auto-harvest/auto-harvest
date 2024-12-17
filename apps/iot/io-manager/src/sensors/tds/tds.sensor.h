#ifndef GRAVITYTDSMETER_H
#define GRAVITYTDSMETER_H

#include <Arduino.h>
#include <map>
#include <string>
#include "sensor.abstract.class.h"
#include "sensors/water-temperature/waterTemperature.sensor.h"

class GravityTDSMeter : public AbstractSensor
{
public:
    GravityTDSMeter(uint8_t pin, WaterTemperatureSensor &tempSensor);
    ~GravityTDSMeter();

    void initialize() override;
    std::map<std::string, double> readData() override;
    const char *getType() override;
    const char *getSensorName() override;

private:
    uint8_t pin;
    WaterTemperatureSensor &tempSensor;
    double readTDS(double temperature);
};

#endif // GRAVITYTDSMETER_H
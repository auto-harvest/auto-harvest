#ifndef DS18B20SENSOR_H
#define DS18B20SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <map>
#include <string>
#include "sensor.abstract.class.h"

class WaterTemperatureSensor : public AbstractSensor
{
public:
    WaterTemperatureSensor(uint8_t pin);
    ~WaterTemperatureSensor();

    void initialize() override;
    std::map<std::string, double> readData() override;
    const char *getType() override;
    const char *getSensorName() override;

private:
    uint8_t pin;
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif // DS18B20SENSOR_H
#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include "sensor.abstract.class.h"
#include <DHT.h>

class DHT11Sensor : public AbstractSensor
{
public:
    DHT11Sensor(uint8_t pin);
    ~DHT11Sensor();
    void initialize() override;
    std::map<std::string, double> readData() override;
    const char *getType() override;
    const char *getSensorName() override;

private:
    uint8_t pin;
    DHT dht;
};

#endif // DHT11_SENSOR_H
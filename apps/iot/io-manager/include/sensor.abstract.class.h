#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H
#include <ArduinoSTL.h>
#include <map>
class AbstractSensor
{
public:
    virtual ~AbstractSensor() = default;
    virtual void initialize() = 0;
    // Pure virtual function to read data from the sensor
    virtual std::map<std::string, double> readData() = 0;

    // Pure virtual function to get the sensor type
    virtual const char *getType() = 0;

    virtual const char *getSensorName() = 0;
};

#endif // SENSOR_INTERFACE_H
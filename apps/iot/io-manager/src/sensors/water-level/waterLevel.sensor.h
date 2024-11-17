#ifndef WATERLEVELSENSOR_H
#define WATERLEVELSENSOR_H

#include <Arduino.h>
#include <map>
#include <string>
#include "sensor.abstract.class.h"

class WaterLevelSensor : public AbstractSensor
{
public:
    WaterLevelSensor(uint8_t lowPin, uint8_t highPin);
    ~WaterLevelSensor();
    void initialize() override;

    std::map<std::string, double> readData() override;
    const char *getType() override;
    const char *getSensorName() override;
    bool isWaterLevelLow();
    bool isWaterLevelAdequate();
    bool isWaterLevelHigh();
    bool isGravityReversed();
    double waterLevelIs();

private:
    uint8_t lowPin;
    uint8_t highPin;
};

#endif // WATERLEVELSENSOR_H
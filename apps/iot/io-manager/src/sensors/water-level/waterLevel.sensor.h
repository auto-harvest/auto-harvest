#ifndef WATERLEVELSENSOR_H
#define WATERLEVELSENSOR_H

#include <Arduino.h>

void initializeWaterLevelSensor();
bool isWaterLevelLow();
bool isWaterLevelAdequate();
bool isWaterLevelHigh();
bool isGravityReversed();
String waterLevelIs();

#endif // WATERLEVELSENSOR_H
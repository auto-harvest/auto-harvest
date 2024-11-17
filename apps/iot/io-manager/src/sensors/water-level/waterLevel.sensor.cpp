#include "waterLevel.sensor.h"

// Define the water level sensor pin
const int waterLevelLowSensorPin = 14;
const int waterLevelHighSensorPin = 15;

void initializeWaterLevelSensor()
{
    // Initialize the water level sensor pin as input
    pinMode(waterLevelLowSensorPin, INPUT_PULLUP);
    pinMode(waterLevelHighSensorPin, INPUT_PULLUP);
}

bool isWaterLevelLow()
{
    // Read the state of the water level sensor pin
    return digitalRead(waterLevelLowSensorPin) == LOW;
}

bool isWaterLevelAdequate()
{
    return digitalRead(waterLevelLowSensorPin) == HIGH && digitalRead(waterLevelHighSensorPin) == LOW;
}

bool isWaterLevelHigh()
{
    return digitalRead(waterLevelHighSensorPin) == HIGH;
}
// in case of gravity flip

bool isGravityReversed()
{
    return digitalRead(waterLevelLowSensorPin) == LOW && digitalRead(waterLevelHighSensorPin) == HIGH;
}

String waterLevelIs(){
    if(isWaterLevelLow()){
        return "low";
    } else if(isWaterLevelAdequate()){
        return "adequate";
    } else if(isWaterLevelHigh()){
        return "high";
    } else if(isGravityReversed()){
        return "oh shoot man";
    }
    return "how did you get here again ?";
}
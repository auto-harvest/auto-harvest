#include "ph.sensor.h"
#include <Arduino.h> // Required for analogRead and millis functions
#define PHADDR 0x00
float neutralVoltage = 1950.0;
float acidVoltage = 1615.0;
// Constructor
PHSensor::PHSensor(int phPin, WaterTemperatureSensor &tempSensor)
    : pin(phPin), temperature(25.0), voltage(0), phValue(0), tempSensor(tempSensor) {}

// Read the voltage and calculate the pH value
float PHSensor::readPH()
{
    // Take multiple samples and average them
    const int numSamples = 50;
    float totalVoltage = 0;

    for (int i = 0; i < numSamples; i++)
    {
        totalVoltage += analogRead(pin);
        delay(10); // Small delay to allow for stable readings
    }

    // Convert averaged ADC value to millivolts
    voltage = (totalVoltage / numSamples) / 1023.0 * 5000.0;
    // Read the analog voltage
    temperature = tempSensor.lastTemperature;

    // Adjust slope based on temperature (Nernst equation)

    float k = 0.33; // sensitivity factor
    float tempSlope = (59.16 + 0.1984 * (temperature - 25.0)) * k; // mV adjustment for given temperature

    float slope = (7.0 - 4.0) / ((neutralVoltage - 1500.0) / tempSlope - (acidVoltage - 1500.0) / tempSlope);
    float intercept = 7.0 - slope * (neutralVoltage - 1500.0) / (tempSlope);

    // Calculate pH
    float rawPh = slope * ((voltage - 1500.0) / tempSlope) + intercept; // y = mx + b

    // Debugging information

    // Serial.print("Temperature: ");
    // Serial.println(temperature);
    // Serial.print("Slope: ");
    // Serial.println(slope);
    // Serial.print("Intercept: ");
    // Serial.println(intercept);
    // Serial.print("pH Value: ");
    // Serial.println(phValue);

    return rawPh;
}

// Perform pH sensor calibration
void PHSensor::calibrate(const char *cmd)
{
    voltage = analogRead(pin) / 1024.0 * 5000;
    Serial.print("Voltage: ");
    Serial.println(voltage);
}

// Set the temperature for compensation
void PHSensor::setTemperature(float temp)
{
    temperature = temp;
}

// Get the temperature used for compensation
float PHSensor::getTemperature() const
{
    return temperature;
}

// Get the last voltage value
float PHSensor::getVoltage() const
{
    return voltage;
}

// Get the last pH value
float PHSensor::getPHValue() const
{
    return phValue;
}

// Implement the initialize method
void PHSensor::initialize()
{
}

// Implement the readData method
std::map<std::string, double> PHSensor::readData()
{
    std::map<std::string, double> data;
    data["ph"] = readPH();
    return data;
}

// Implement the getType method
const char *PHSensor::getType()
{
    return "ph";
}

// Implement the getSensorName method
const char *PHSensor::getSensorName()
{
    return "SEN0161";
}

#include "ph.sensor.h"
#include <Arduino.h> // Required for analogRead and millis functions
#define PHADDR 0x00
float neutralVoltage = 1900.0;
float acidVoltage = 1600.0;
// Constructor
PHSensor::PHSensor(int phPin, WaterTemperatureSensor& tempSensor)
    : pin(phPin), temperature(25.0), voltage(0), phValue(0), tempSensor() {}

// Read the voltage and calculate the pH value
float PHSensor::readPH()
{
    // Read the analog voltage
    voltage = analogRead(pin) / 1024.0 * 5000.0; // Convert ADC value to millivolts
    temperature = 19.0;

    // Adjust slope based on temperature (Nernst equation)
    float tempSlope = 59.16 + 0.1984 * (temperature - 25.0); // Adjust slope in mV/pH per °C
    float slope = (7.0 - 4.0) / ((neutralVoltage - 1500.0) / (tempSlope / 3.0) - (acidVoltage - 1500.0) / (tempSlope / 3.0));
    float intercept = 7.0 - slope * (neutralVoltage - 1500.0) / (tempSlope / 3.0);

    // Calculate pH
    float phValue = slope * (voltage - 1500.0) / (tempSlope / 3.0) + intercept; // y = mx + b

    // Debugging information
    Serial.print("Voltage: ");
    Serial.println(voltage);
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Slope: ");
    Serial.println(slope);
    Serial.print("Intercept: ");
    Serial.println(intercept);
    Serial.print("pH Value: ");
    Serial.println(phValue);

    return phValue;
}

// Perform pH sensor calibration
void PHSensor::calibrate(const char *cmd)
{
    voltage = analogRead(pin) / 1024.0 * 5000;
    Serial.print("Voltage: ");
    Serial.println(voltage);

    ph->calibration(voltage, temperature, cmd);
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

    ph->begin();
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
    return "Gravity PH Sensor v1";
}

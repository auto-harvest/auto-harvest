#include "tds.sensor.h"

GravityTDSMeter::GravityTDSMeter(uint8_t pin, WaterTemperatureSensor *tempSensor) : pin(pin), tempSensor(tempSensor)
{
}

GravityTDSMeter::~GravityTDSMeter()
{
    // Destructor implementation (if needed)
}

void GravityTDSMeter::initialize()
{
    pinMode(pin, INPUT);
}

std::map<std::string, double> GravityTDSMeter::readData()
{
    std::map<std::string, double> data;
    double temperature = tempSensor->readData()["water-temperature"];
    double tdsValue = readTDS(temperature);
    data["tds"] = tdsValue;
    return data;
}

const char *GravityTDSMeter::getType()
{
    return "GravityTDS";
}

const char *GravityTDSMeter::getSensorName()
{
    return "Gravity TDS Meter";
}

double GravityTDSMeter::readTDS(double temperature)
{
    // Read the analog value from the TDS sensor
    int analogValue = analogRead(pin);

    // Convert the analog value to voltage
    double voltage = analogValue * (5.0 / 1024.0);

    // Calculate the TDS value using the voltage and temperature
    double compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    double compensationVoltage = voltage / compensationCoefficient;
    double tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5; // TDS conversion factor

    return tdsValue;
}
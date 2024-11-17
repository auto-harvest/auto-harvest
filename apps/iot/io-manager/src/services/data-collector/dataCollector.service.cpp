#include "dataCollector.service.h"

#include <Arduino.h>

DataCollector::DataCollector()
{
    // Add sensors to the vector
    auto waterTempSensor = new WaterTemperatureSensor(17); //  DS18B20 is connected to pin 17
    sensors.push_back(new DHT11Sensor(16));                //  DHT11 is connected to pin 16
    sensors.push_back(new WaterLevelSensor(14, 15));       //  water level sensors are connected to pins 14 and 15
    sensors.push_back(waterTempSensor);
    sensors.push_back(new GravityTDSMeter(A7, waterTempSensor));
    // Add other sensors similarly
}

DataCollector::~DataCollector()
{
    // Manually delete the sensors to free memory
    for (auto sensor : sensors)
    {
        delete sensor;
    }
}

void DataCollector::initializeSensors()
{
    // Initialize each sensor if needed
    for (auto sensor : sensors)
    {
        sensor->initialize();
    }
}

std::map<std::string, double> DataCollector::collectData()
{
    std::map<std::string, double> collectedData;
    for (auto sensor : sensors)
    {
        auto data = sensor->readData();
        collectedData.insert(data.begin(), data.end());
    }
    return collectedData;
}

void DataCollector::printData(const std::map<std::string, double> data)
{
    for (const auto &entry : data)
    {
        Serial.print(entry.first.c_str());
        Serial.print(": ");
        Serial.println(entry.second);
    }
}
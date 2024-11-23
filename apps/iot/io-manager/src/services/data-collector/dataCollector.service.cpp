#include "dataCollector.service.h"

#include <Arduino.h>

DataCollector *DataCollector::instance = nullptr;

DataCollector::DataCollector()
{
    // Add sensors to the vector

    // Add other sensors similarly
    instance = this;
}

DataCollector::~DataCollector()
{
    instance = nullptr;
    // Manually delete the sensors to free memory
    for (auto sensor : sensors)
    {
        delete sensor;
    }
}

void DataCollector::initializeSensors()
{
    auto waterTempSensor = new WaterTemperatureSensor(17); //  DS18B20 is connected to pin 17
   // auto phSensor = new PHSensor(A6, waterTempSensor);
    // sensors.push_back(waterTempSensor);
    // sensors.push_back(new DHT11Sensor(16));          //  DHT11 is connected to pin 16
    // sensors.push_back(new WaterLevelSensor(14, 15)); //  water level sensors are connected to pins 14 and 15
    sensors.push_back(new GravityTDSMeter(A7, waterTempSensor));
   // sensors.push_back(phSensor);                //  pH sensor is connected to pin A6
    sensors.push_back(new WaterFlowSensor(40)); //  water flow sensor is connected to pin 18
    // Initialize each sensor if needed
    for (auto sensor : sensors)
    {
        sensor->initialize();
        delay(100);
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
    for (auto &entry : collectedData)
    {
        previousData[entry.first] = currentData[entry.first];
    }
    currentData = collectedData;
    return collectedData;
}

void DataCollector::printData(const std::map<std::string, double> data)
{
    Serial.println("--------------------");
    for (const auto &entry : data)
    {
        Serial.print(entry.first.c_str());
        Serial.print(": ");
        Serial.println(entry.second);
    }
    Serial.println("--------------------");
}
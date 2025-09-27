#include "dataCollector.service.h"

#include <Arduino.h>
#include "context/app.context.h"
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
    auto phSensor = new PHSensor(A6, *waterTempSensor);
    sensors.push_back(waterTempSensor);
    sensors.push_back(new DHT11Sensor(16));          //  DHT11 is connected to pin 16
    sensors.push_back(new WaterLevelSensor(14, 15)); //  water level sensors are connected to pins 14 and 15
    sensors.push_back(new GravityTDSMeter(A7, *waterTempSensor));
    sensors.push_back(phSensor);               //  pH sensor is connected to pin A6
    sensors.push_back(new WaterFlowSensor(2)); //  water flow sensor is connected to pin 18
    // Initialize each sensor if needed
    for (auto sensor : sensors)
    {
        sensor->initialize();
        delay(100);
    }
}

std::map<std::string, double> DataCollector::collectData()
{
    std::map<std::string, double> data;
    for (auto sensor : sensors)
    {
        auto sensorData = sensor->readData();
        for (const auto &entry : sensorData)
        {
            data[entry.first] = entry.second;
        }
    }
    // app context module manager
    AppContext &appContext = AppContext::getInstance();
    data["air-pump"] = (std::string(appContext.moduleManager->airPump->getStatus()) == "On" ? 1.0 : 0.0);
    data["water-pump"] = (std::string(appContext.moduleManager->waterPump->getStatus()) == "On" ? 1.0 : 0.0);
    // to lowercase
    for (auto &entry : data)
    {
        std::string key = entry.first;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if (key != entry.first)
        {
            data[key] = entry.second;
            data.erase(entry.first);
        }
    }
    previousData = currentData;
    currentData = data;
    return data;
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

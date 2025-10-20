

#ifndef DATA_COLLECTOR_SERVICE_H
#define DATA_COLLECTOR_SERVICE_H

#include "sensor.abstract.class.h"
#include "sensors/humidity/humidity.sensor.h"
#include "sensors/water-level/waterLevel.sensor.h"
#include "sensors/water-temperature/waterTemperature.sensor.h"
#include "sensors/tds/tds.sensor.h"
#include "sensors/ph/ph.sensor.h"
#include "sensors/water-flow/waterFlow.sensor.h"
#include <ArduinoSTL.h>
#include <map>
#include <memory>
#include <vector>
#include <string>

class DataCollector
{
public:
    DataCollector();
    ~DataCollector();

    void initializeSensors();
    std::map<std::string, double> collectData();
    static DataCollector *getInstance();
    void printData(const std::map<std::string, double> data);
    String getStatus();
    std::map<std::string, double> currentData;

    std::map<std::string, double> previousData;
    PHSensor *phSensor;

private:
    static DataCollector *instance;
    std::vector<AbstractSensor *> sensors;
    String status;
};

#endif // DATA_COLLECTOR_SERVICE_H
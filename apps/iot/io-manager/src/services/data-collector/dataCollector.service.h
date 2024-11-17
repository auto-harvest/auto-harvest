

#ifndef DATA_COLLECTOR_SERVICE_H
#define DATA_COLLECTOR_SERVICE_H

#include "sensor.abstract.class.h"
#include "sensors/humidity/humidity.sensor.h"
#include "sensors/water-level/waterLevel.sensor.h"
#include "sensors/water-temperature/waterTemperature.sensor.h"
#include "sensors/tds/tds.sensor.h"

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
    void printData(const std::map<std::string, double> data);

private:
    std::vector<AbstractSensor *> sensors;
};

#endif // DATA_COLLECTOR_SERVICE_H
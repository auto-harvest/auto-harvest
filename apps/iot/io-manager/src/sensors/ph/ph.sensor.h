#ifndef PH_SENSOR_H
#define PH_SENSOR_H
#include "sensors/water-temperature/waterTemperature.sensor.h"
#include "sensor.abstract.class.h"

class PHSensor : public AbstractSensor
{
private:
    int pin;           // Analog pin for the pH sensor
    float temperature; // Temperature value for compensation
    float voltage;     // Voltage value read from the sensor
    float phValue;     // Calculated pH value
    WaterTemperatureSensor &tempSensor;

public:
    // Constructor to initialize the pin and default temperature
    PHSensor(int phPin, WaterTemperatureSensor &tempSensor);

    // Initialize the pH sensor
    void begin();

    // Read the voltage and calculate the pH value
    float readPH();

    // Perform pH sensor calibration
    void calibrate(const char *cmd);

    // Set the temperature for compensation
    void setTemperature(float temp);

    // Get the temperature used for compensation
    float getTemperature() const;

    // Get the last voltage value
    float getVoltage() const;

    // Get the last pH value
    float getPHValue() const;

    // Implement the initialize method
    void initialize() override;

    // Implement the readData method
    std::map<std::string, double> readData() override;

    // Implement the getType method
    const char *getType() override;

    // Implement the getSensorName method
    const char *getSensorName() override;
};

#endif // PH_SENSOR_H

#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H
#include <ArduinoSTL.h>
#include <map>
class AbstractSensor
{
public:
    virtual ~AbstractSensor() = default;
    virtual void initialize() = 0;
    virtual std::map<std::string, double> readData() = 0;
    virtual const char *getType() = 0;
    virtual const char *getSensorName() = 0;

    virtual void calibrate()
    {
        log("Default calibration executed.");
    }

    unsigned long getTimeElapsedSinceLastRead() const
    {
        return timeElapsedSinceLastRead;
    }

    unsigned long getReadCount() const
    {
        return readCount;
    }

    std::string getStatus() const
    {
        return statusMessage;
    }

    void setUniqueID(const std::string &id)
    {
        uniqueID = id;
    }

    std::string getUniqueID() const
    {
        return uniqueID;
    }

    std::map<std::string, double> readDataWithMetrics()
    {
        resetReadTime();
        auto data = readData();
        incrementReadCount();
        updateReadTime();
        log(("Time elapsed since last read: " + String(timeElapsedSinceLastRead) + " ms").c_str());

        return data;
    }

protected:
    unsigned long lastReadTime = 0;
    unsigned long timeElapsedSinceLastRead = 0;
    unsigned long readCount = 0;

    std::string statusMessage = "OK";
    std::string uniqueID;

    void resetReadTime()
    {
        lastReadTime = millis();
        timeElapsedSinceLastRead = 0;
    }

    void updateReadTime()
    {
        unsigned long currentTime = millis();
        if (lastReadTime != 0)
        {
            timeElapsedSinceLastRead = currentTime - lastReadTime;
        }
        lastReadTime = currentTime;
    }

    void incrementReadCount()
    {
        readCount++;
    }

    void log(const std::string &message)
    {
        Serial.println(("[" + std::string(getSensorName()) + "] " + message).c_str());
    }

    void setStatus(const std::string &status)
    {
        statusMessage = status;
    }
};
#endif // SENSOR_INTERFACE_H
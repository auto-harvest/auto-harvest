#include "humidity.sensor.h"

DHT11Sensor::DHT11Sensor(uint8_t pin) : pin(pin), dht(pin, DHT11)
{
}

void DHT11Sensor::initialize()
{
    Serial.println("Initializing DHT11 Sensor...");
    dht.begin();
}

DHT11Sensor::~DHT11Sensor()
{
    // Destructor implementation (if needed)
}

std::map<std::string, double> DHT11Sensor::readData()
{
    std::map<std::string, double> data;
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if any reads failed and return empty map if so
    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Failed to read from DHT sensor!");
        return data;
    }

    data["t"] = temperature;
    data["h"] = humidity;
    return data;
}

const char *DHT11Sensor::getType()
{
    return "DHT11";
}

const char *DHT11Sensor::getSensorName()
{
    return "DHT11";
}
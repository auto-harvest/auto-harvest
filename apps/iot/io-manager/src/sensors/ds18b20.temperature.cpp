#include <DS18B20.h>

DS18B20 sensor(14); // Initialize sensor at pin 2
float getTemperature();

bool requestTemperature()
{
    sensor.requestTemperatures(); // Send the command to get temperatures
    float temp = sensor.getTempC();
     return temp;
    
}
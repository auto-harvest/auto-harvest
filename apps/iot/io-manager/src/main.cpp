#include <Arduino.h>
#include "sensors/water-level/waterLevel.sensor.h"
#include "relay/relay.module.h"
#include <Wire.h>
#include "Waveshare_LCD1602.h"

Waveshare_LCD1602 lcd(16, 2); // 16 characters and 2 lines of show
void show(char *str);
String previous = "";
int i = 0;
void setup()
{
    // Initialize serial communication for debugging
    Serial.begin(115200);
    // initialize
    Wire.begin();
    Serial.println("Scanning for I2C devices...");

    Serial.println("Scanning complete.");
    lcd.init();

    lcd.setCursor(0, 0);
    lcd.send_string("AutoHarvest v0.1");
    lcd.setCursor(0, 1);
    lcd.send_string("Initializing...");
    lcd.blink();
    initializeWaterLevelSensor();

    // Initialize the relays
    initializeRelays();
    turnOnRelay(relayPins[0]);
    turnOnRelay(relayPins[1]);
}
int r, g, b, t = 0;

void loop()
{
    // Check the state of the water level sensor

    // Serial.println("Water level is: " + waterLevelIs());
    Serial.println("Hello");
    i += 1;
    // Convert the integer to a character array
    char buffer[10];
    itoa(i, buffer, 10); // Convert integer to string (base 10)
    show(buffer);
    delay(1000); // Wait for 1 second before checking again
}

void show(char *str)
{
    if (str == previous.c_str())
    {
        return;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.send_string(str);
    lcd.setCursor(0, 1);
    lcd.send_string("Hello marie!");
    previous = str;
}
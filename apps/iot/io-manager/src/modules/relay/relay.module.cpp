#include "relay.module.h"

// Define the relay pins
const int relayPins[] = {18, 19, 20, 21, 22};

void initializeRelays()
{
    // Initialize the relay pins as outputs
    for (int i = 0; i <= 4; i++)
    { 
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], HIGH); // Initially turn off all relays
    }
}

void turnOnRelay(int relayPin)
{
    digitalWrite(relayPin, LOW);
}

void turnOffRelay(int relayPin)
{
    digitalWrite(relayPin, HIGH);
}
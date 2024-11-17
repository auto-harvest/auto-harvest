#include "relay.module.h"

// Constructor
RelayModule::RelayModule(const int *pins, int numPins) : pins(pins), numPins(numPins)
{
}

// Destructor
RelayModule::~RelayModule()
{
    // Destructor implementation (if needed)
}

// Initialize the relay pins as outputs
void RelayModule::initialize()
{
    for (int i = 0; i < numPins; i++)
    {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], HIGH); // Initially turn off all relays
    }
}

// Turn on the relay
void RelayModule::turnOn(int relayPin)
{
    digitalWrite(relayPin, LOW);
}

// Turn off the relay
void RelayModule::turnOff(int relayPin)
{
    digitalWrite(relayPin, HIGH);
}
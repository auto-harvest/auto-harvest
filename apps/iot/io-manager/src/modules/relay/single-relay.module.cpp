#include "single-relay.module.h"

// Constructor
SingleRelay::SingleRelay(uint8_t relayPin) : relayPin(relayPin), powerState(false)
{
}

// Destructor
SingleRelay::~SingleRelay()
{
    // Destructor implementation (if needed)
}

// Initialize the relay pin as output
void SingleRelay::initialize()
{
    Serial.print("Initializing Single Relay... ");
    Serial.println(relayPin);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Initially turn off the relay
}

// Turn on the relay
void SingleRelay::turnOn()
{
    digitalWrite(relayPin, LOW);
    powerState = true;
}

// Turn off the relay
void SingleRelay::turnOff()
{
    digitalWrite(relayPin, HIGH);
    powerState = false;
}

// Get the type of the relay
const char *SingleRelay::getType()
{
    return "Relay";
}

// Get the name of the relay
const char *SingleRelay::getName()
{
    return "Single Relay";
}

// Get the status of the relay
const char *SingleRelay::getStatus()
{
    return powerState ? "On" : "Off";
}

// Set the power state of the relay
void SingleRelay::setPower(bool power)
{
    powerState = power;
    if (power)
    {
        turnOn();
    }
    else
    {
        turnOff();
    }
}
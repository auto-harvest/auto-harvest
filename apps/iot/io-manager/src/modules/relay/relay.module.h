
#ifndef RELAY_MODULE_H
#define RELAY_MODULE_H

#include <Arduino.h>

class RelayModule
{
public:
    RelayModule(const int *pins, int numPins);
    ~RelayModule();

    void initialize();
    void turnOn(int relayPin);
    void turnOff(int relayPin);

private:
    const int *pins;
    int numPins;
};

#endif // RELAY_MODULE_H
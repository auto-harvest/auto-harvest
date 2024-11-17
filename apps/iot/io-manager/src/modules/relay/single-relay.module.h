#ifndef SINGLE_RELAY_H
#define SINGLE_RELAY_H
#include "module.abstract.class.h"

class SingleRelay : public AbstractModule
{
public:
    SingleRelay(uint8_t relayPin);
    ~SingleRelay();

    void initialize() override;
    const char *getType() override;
    const char *getName() override;
    const char *getStatus() override;
    void setPower(bool power) override;

    void turnOn();
    void turnOff();

private:
    uint8_t relayPin;
    bool powerState;
};

#endif // SINGLE_RELAY_H
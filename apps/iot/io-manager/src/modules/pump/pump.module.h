#ifndef PUMP_MODULE_H
#define PUMP_MODULE_H

#include "module.abstract.class.h"
#include "modules/relay/single-relay.module.h" // Include the header file for SingleRelay class

class PumpModule : public AbstractModule
{
public:
    PumpModule(SingleRelay *relay);
    ~PumpModule();
    void initialize() override;
    const char *getType() override;
    const char *getName() override;
    const char *getStatus() override;
    void setPower(bool power) override;

private:
    SingleRelay *relay;
    bool powerState;
};

#endif // PUMP_MODULE_H
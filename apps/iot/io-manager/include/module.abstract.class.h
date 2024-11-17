#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

#include <ArduinoSTL.h>
#include <string>

class AbstractModule
{
public:
    virtual ~AbstractModule() = default;

    // Pure virtual function to initialize the module
    virtual void initialize() = 0;

    // Pure virtual function to get the module type
    virtual const char *getType() = 0;

    // Pure virtual function to get the module name
    virtual const char *getName() = 0;

    // Pure virtual function to get the module status
    virtual const char *getStatus() = 0;

    // Pure virtual function to set the power state of the module
    virtual void setPower(bool power) = 0;
};

#endif // MODULE_INTERFACE_H
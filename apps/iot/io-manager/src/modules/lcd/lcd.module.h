#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include "Waveshare_LCD1602.h"
#include "module.abstract.class.h"
#include "services/data-collector/dataCollector.service.h"

class LCDModule : public AbstractModule
{
public:
    LCDModule(DataCollector *dataCollector);
    ~LCDModule();

    void initialize() override;
    void update(const char *status);
    void displaySensorData();
    const char *getType() override;
    const char *getName() override;
    const char *getStatus() override;
    void setPower(bool power) override;

private:
    Waveshare_LCD1602 *screen;
    DataCollector *dataCollector;
    String status;
};

#endif // LCD_MODULE_H
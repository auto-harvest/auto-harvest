#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include <Arduino.h>
#include <queue>
#include <string>
#include "Waveshare_LCD1602.h"
#include "services/data-collector/dataCollector.service.h"

class LCDModule
{
public:
    LCDModule(DataCollector *dataCollector);
    ~LCDModule();

    void initialize();
    void processMessageQueue();
    void addMessageToQueue(const String &message);
    void updateCarousel();
    void displayPage(int pageIndex);
    void displaySensorData();
    void displayUptime();
    void displayConnectionStatus();
    void displayDeviceStatus();
    void setPower(bool power);
    const char *getType();
    const char *getName();
    const char *getStatus();

private:
    Waveshare_LCD1602 *screen;
    DataCollector *dataCollector;
    String status;
    std::queue<String> messageQueue;
    unsigned long lastUpdateMillis;
    unsigned long carouselDelay;
    unsigned long uptimeStartMillis;
    int currentPageIndex;
    static const int maxQueueSize = 10;

    int getPageCount() const { return 4; }
};

#endif // LCD_MODULE_H
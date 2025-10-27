#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include <Arduino.h>
#include <queue>
#include <string>
#include <WiFiEspAT.h>
#include "Waveshare_LCD1602.h"
#include "services/data-collector/dataCollector.service.h"
#include "services/wifi-manager/wifiManager.service.h"
#include "services/activeMQ-client/activeMQ-client.service.h"

class LCDModule
{
public:
    LCDModule(DataCollector *dataCollector, WiFiService *wifiService, ActiveMQClientService *mqttService = nullptr);
    ~LCDModule();

    void initialize();
    void processMessageQueue();
    void addMessageToQueue(const String &message);
    void updateCarousel();
    void displayPage(int pageIndex);

    // Carousel pages
    void displaySystemInfo();           // Page 0: System name + WiFi status
    void displayWaterQuality();         // Page 1: pH + TDS
    void displayTemperatureHumidity();  // Page 2: Temp + Humidity
    void displayFlowData();             // Page 3: Flow rate + Total liters
    void displayUptime();               // Page 4: Uptime
    void displayDeviceStatus();         // Page 5: Device status
    void displayDateTime();             // Page 6: Date and Time
    void displayServiceStatus();        // Page 7: Service statuses

    void setPower(bool power);
    const char *getType();
    const char *getName();
    const char *getStatus();

    // Time functions
    void setTime(const String &time, const String &date);

private:
    Waveshare_LCD1602 *screen;
    DataCollector *dataCollector;
    WiFiService *wifiService;
    ActiveMQClientService *mqttService;
    String status;
    std::queue<String> messageQueue;
    unsigned long lastUpdateMillis;
    unsigned long carouselDelay;
    unsigned long uptimeStartMillis;
    int currentPageIndex;
    static const int maxQueueSize = 10;

    // Time tracking (just store pre-formatted strings from server)
    bool timeSynced = false;
    String currentTime = "--:--:--";
    String currentDate = "--/--/----";

    int getPageCount() const { return 7; } // 7 pages in carousel
    String getWiFiStatus();
    String centerText(String text, int width);
};

#endif // LCD_MODULE_H
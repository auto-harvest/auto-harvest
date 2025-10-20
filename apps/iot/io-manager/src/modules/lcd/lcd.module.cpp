#include "lcd.module.h"
#include <queue>
#include <string>

// Constructor
LCDModule::LCDModule(DataCollector *dataCollector, WiFiService *wifiService, ActiveMQClientService *mqttService)
    : dataCollector(dataCollector), wifiService(wifiService), mqttService(mqttService), status("Initializing..."), lastUpdateMillis(0), carouselDelay(3000), uptimeStartMillis(millis()), currentPageIndex(0)
{
    screen = new Waveshare_LCD1602(16, 2);
}

// Destructor
LCDModule::~LCDModule()
{
    delete screen; // Clean up the allocated screen object
}

// Initialize the LCD module
void LCDModule::initialize()
{
    Serial.println("Initializing LCD Module...");
    screen->init();

    screen->setCursor(0, 0);
    screen->send_string("AutoHarvest v0.111");
    screen->send_string(status.c_str());
    screen->blink();

    // Update status after initialization
    status = "Ready";
}

// Update the LCD display for the message queue
void LCDModule::processMessageQueue()
{
    if (!messageQueue.empty())
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastUpdateMillis >= carouselDelay)
        {
            lastUpdateMillis = currentMillis;

            // Display the next message
            String message = messageQueue.front();
            messageQueue.pop();

            screen->setCursor(0, 0);
            screen->send_string("                "); // Clear the line
            screen->send_string(message.c_str());

            // Add a delay before removing the last message
            if (!messageQueue.empty())
            {
                messageQueue.push(message); // Re-add to maintain FIFO for messages
            }
        }
    }
}

// Add a message to the queue
void LCDModule::addMessageToQueue(const String &message)
{
    if (messageQueue.size() < maxQueueSize)
    {
        messageQueue.push(message);
    }
}

// Update the LCD display carousel
void LCDModule::updateCarousel()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdateMillis >= carouselDelay)
    {
        lastUpdateMillis = currentMillis;
    displayPage(currentPageIndex);

        // Cycle through pages
        currentPageIndex = (currentPageIndex + 1) % getPageCount();
    }
}

// Display the specified page
void LCDModule::displayPage(int pageIndex)
{
    screen->clear();
    screen->setCursor(0, 0);

    switch (pageIndex)
    {
    case 0: // System name + WiFi status
        displaySystemInfo();
        break;
    case 1: // pH + TDS
        displayWaterQuality();
        break;
    case 2: // Temperature + Humidity
        displayTemperatureHumidity();
        break;
    case 3: // Flow rate + Liters
        displayFlowData();
        break;
    case 4: // Uptime
        displayUptime();
        break;
    case 5: // Device status
        displayDeviceStatus();
        break;
    case 6: // Date and Time
        displayDateTime();
        break;
    case 7: // Service statuses
        displayServiceStatus();
        break;
    }
}

// Page 0: System Info with WiFi status
void LCDModule::displaySystemInfo()
{
    screen->setCursor(0, 0);
    screen->send_string(" AutoHarvest   "); // 16 chars max

    screen->setCursor(0, 1);
    String wifiStatus = getWiFiStatus();
    screen->send_string(wifiStatus.c_str());
}

// Page 1: Water Quality (pH + TDS)
void LCDModule::displayWaterQuality()
{
    auto data = dataCollector->currentData;

    screen->setCursor(0, 0);
    String phLine = "pH:" + String(data["ph"], 2); // Compact format
    screen->send_string(phLine.c_str());

    screen->setCursor(0, 1);
    String tdsLine = "TDS:" + String(data["tds"], 0) + "ppm"; // Compact format
    screen->send_string(tdsLine.c_str());
}

// Page 2: Temperature & Humidity
void LCDModule::displayTemperatureHumidity()
{
    auto data = dataCollector->currentData;

    screen->setCursor(0, 0);
    String tempLine = "Temp:" + String(data["t"], 1) + "C"; // Compact format
    screen->send_string(tempLine.c_str());

    screen->setCursor(0, 1);
    String humLine = "Humid:" + String(data["h"], 0) + "%"; // Compact format
    screen->send_string(humLine.c_str());
}

// Page 3: Flow Data
void LCDModule::displayFlowData()
{
    auto data = dataCollector->currentData;

    screen->setCursor(0, 0);
    screen->send_string("Water Flow      ");

    screen->setCursor(0, 1);
    String litersLine = String(data["lpm"], 2) + " L/min";
    String centeredLine = centerText(litersLine, 16);
    screen->send_string(centeredLine.c_str());
}

// Page 4: Uptime
void LCDModule::displayUptime()
{
    screen->setCursor(0, 0);
    screen->send_string("Uptime");

    screen->setCursor(0, 1);
    unsigned long uptimeMillis = millis() - uptimeStartMillis;
    unsigned long seconds = (uptimeMillis / 1000UL) % 60;
    unsigned long minutes = (uptimeMillis / 60000UL) % 60;
    unsigned long hours = (uptimeMillis / 3600000UL);

    String uptimeString = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
    screen->send_string(uptimeString.c_str());
}

// Page 5: Device Status
void LCDModule::displayDeviceStatus()
{
    screen->setCursor(0, 0);
    screen->send_string("Status");

    screen->setCursor(0, 1);
    screen->send_string(status.c_str());
}

// Page 6: Date and Time
void LCDModule::displayDateTime()
{
    screen->setCursor(0, 0);
    if (timeSynced)
    {
        // Display pre-formatted time from server
        String timeLine = centerText(currentTime, 16);
        screen->send_string(timeLine.c_str());

        screen->setCursor(0, 1);
        String dateLine = centerText(currentDate, 16);
        screen->send_string(dateLine.c_str());
    }
    else
    {
        screen->send_string("Time Not Synced ");
        screen->setCursor(0, 1);
        screen->send_string("Waiting...      ");
    }
}

// Set time from MQTT server (receives pre-formatted strings)
void LCDModule::setTime(const String &time, const String &date)
{
    currentTime = time;
    currentDate = date;
    timeSynced = true;
}

// Helper: Get WiFi connection status
String LCDModule::getWiFiStatus()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return "WiFi:Connected  "; // 16 chars
    }
    else if (wifiService->mode == "ap")
    {
        return "WiFi: AP Mode   "; // 16 chars
    }
    else
    {
        return "WiFi:No Connect "; // 16 chars (shortened "Disconnected" to "No Connect")
    }
}

// Helper: Center text on LCD (16 char width)
String LCDModule::centerText(String text, int width)
{
    int textLen = (int)text.length();
    int padding = (width - textLen) / 2;
    String result = "";
    for (int i = 0; i < padding; i++)
    {
        result += " ";
    }
    result += text;
    while ((int)result.length() < width)
    {
        result += " ";
    }
    return result;
}

// Set the power state of the module
void LCDModule::setPower(bool power)
{
    status = power ? "ON" : "OFF";
}

// Get the type of the module
const char *LCDModule::getType()
{
    return "LCD";
}

// Get the name of the module
const char *LCDModule::getName()
{
    return "LCD Module";
}

// Get the status of the module
const char *LCDModule::getStatus()
{
    return status.c_str();
}

// Page 7: Service Status
void LCDModule::displayServiceStatus()
{
    screen->setCursor(0, 0);

    // Line 1: WiFi and Sensors status
    String wifiStat = wifiService->getStatus();
    String sensorStat = dataCollector->getStatus();

    // Abbreviate status names for compact display
    if (wifiStat == "Connected") wifiStat = "Conn";
    else if (wifiStat == "Connecting") wifiStat = "Conn...";
    else if (wifiStat == "Disconnected") wifiStat = "Disc";
    else if (wifiStat == "AP Mode") wifiStat = "AP";
    else if (wifiStat == "Ready") wifiStat = "Rdy";

    if (sensorStat == "Active") sensorStat = "Act";
    else if (sensorStat == "Initializing") sensorStat = "Init";

    String line1 = "W:" + wifiStat + " S:" + sensorStat;
    screen->send_string(line1.c_str());

    screen->setCursor(0, 1);

 
}

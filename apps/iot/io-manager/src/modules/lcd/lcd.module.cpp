#include "lcd.module.h"
#include <queue>
#include <string>

// Constructor
LCDModule::LCDModule(DataCollector *dataCollector)
    : dataCollector(dataCollector), status("Initializing..."), currentPageIndex(0), lastUpdateMillis(0), carouselDelay(3000), uptimeStartMillis(millis())
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

        // Cycle through pages
        displayPage(currentPageIndex);
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
    case 0: // Display sensor data
        displaySensorData();
        break;
    case 1: // Display uptime
        displayUptime();
        break;
    case 2: // Display connection status
        displayConnectionStatus();
        break;
    case 3: // Display device status
        displayDeviceStatus();
        break;
    }
}

// Display sensor data
void LCDModule::displaySensorData()
{
    screen->setCursor(0, 1);
    auto data = dataCollector->currentData;
    String temperature = String(data["water-temperature"], 2) + "C " + String(data["tds"], 0) + "ppm";
    screen->send_string(temperature.c_str());
}

// Display uptime
void LCDModule::displayUptime()
{
    screen->setCursor(0, 1);
    unsigned long uptimeMillis = millis() - uptimeStartMillis;
    unsigned long seconds = (uptimeMillis / 1000) % 60;
    unsigned long minutes = (uptimeMillis / (1000 * 60)) % 60;
    unsigned long hours = (uptimeMillis / (1000 * 60 * 60));

    String uptimeString = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
    screen->send_string(uptimeString.c_str());
}

// Display connection status
void LCDModule::displayConnectionStatus()
{
    screen->setCursor(0, 1);
    String connectionStatus = "Connected: 100%";
    screen->send_string(connectionStatus.c_str());
}

// Display device status
void LCDModule::displayDeviceStatus()
{
    screen->setCursor(0, 1);
    screen->send_string(("Status: " + status).c_str());
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

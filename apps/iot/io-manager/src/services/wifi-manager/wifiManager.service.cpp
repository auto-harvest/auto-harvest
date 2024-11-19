#include "wifiManager.service.h"

WiFiService::WiFiService(DiskManagerService *diskManager) : diskManager(diskManager) {}

void WiFiService::begin()
{
    Serial1.begin(115200);
    WiFi.init(&Serial1); // Initialize the WiFi module with the Serial1 interface
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        while (true)
            ; // Don't continue
    }
    WiFi.disconnect();
}

void WiFiService::scanNetworks()
{
    int n = WiFi.scanNetworks();
    Serial.println("Scan completed");
    if (n == 0)
    {
        Serial.println("No networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            delay(10);
        }
    }
}

void WiFiService::turnToAccessPointMode(const char *ssid, const char *password)
{

    WiFi.softAP(ssid, password, 10);
    WiFi.config(IPAddress(5, 5, 5, 5), IPAddress(5, 5, 5, 5), IPAddress(255, 255, 255, 0));
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("Channel: ");
    Serial.println(WiFi.channel());

    Serial.print("DHCP Enabled: ");
    Serial.println(WiFi.dhcpIsEnabled());
}

void WiFiService::turnToNormalMode()
{
    WiFi.disconnect();
    Serial.println("Normal Mode");
}

void WiFiService::connectToWiFi(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
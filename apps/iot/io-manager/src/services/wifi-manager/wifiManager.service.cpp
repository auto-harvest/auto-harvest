#include "wifiManager.service.h"

WiFiService::WiFiService() : status("Initializing") {}

String WiFiService::begin()
{
    Serial1.begin(115200);
    status = "Initializing";
    do
    {
        WiFi.init(&Serial1); // Initialize the WiFi module with the Serial1 interface
        if (WiFi.status() == WL_NO_SHIELD)
        {
            Serial.println("WiFi shield not present");
            status = "No Shield";
            delay(1000);
        }
    } while (WiFi.status() == WL_NO_SHIELD);

    WiFi.disconnect();
    status = "Ready";
    return getMacAddress();
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
    // WiFi.disconnect();

    WiFi.softAP(ssid, password, 10);
    WiFi.softAPConfig(IPAddress(5, 5, 5, 5), IPAddress(5, 5, 5, 5), IPAddress(255, 255, 255, 0));
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    this->mode = "ap";
    this->status = "AP Mode";
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
    this->mode = "client";
    this->status = "Disconnected";
    // close access point
    WiFi.softAPdisconnect();
}

void WiFiService::connectToWiFi(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    this->status = "Connecting";

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    this->mode = "client";
    this->status = "Connected";
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.softAPdisconnect();
}
String WiFiService::getMacAddress()
{
    byte mac[6];
    WiFi.macAddress(mac);
    char macStr[18] = {0};
    for (int i = 0; i < 6; ++i)
    {
        if (i > 0)
            strcat(macStr, ":");
        char octet[3];
        sprintf(octet, "%02X", mac[i]);
        strcat(macStr, octet);
    }
    Serial.print("MAC Address: ");
    Serial.println(macStr);
    return macStr;
}

String WiFiService::getStatus()
{
    return status;
}
bool WiFiService::connectToHost(const char *hostname, uint16_t port, IPAddress fallbackIP, WiFiClient &client)
{
    IPAddress serverIP;
    Serial.print("Resolving hostname: ");
    Serial.println(hostname);

    if (WiFi.hostByName(hostname, serverIP))
    {
        Serial.print("DNS resolved: ");
        Serial.println(serverIP);
    }
    else
    {
        Serial.println("DNS failed. Using fallback IP.");
        serverIP = fallbackIP; // <- optional fallback
    }

    Serial.print("Connecting to server: ");
    Serial.println(serverIP);

    if (client.connect(serverIP, port))
    {
        Serial.println("Connected to server!");
        return true;
    }
    else
    {
        Serial.println("Connection failed!");
        return false;
    }
}
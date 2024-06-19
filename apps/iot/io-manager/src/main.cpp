#include <SPI.h>
#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>

void reconnect();
void publishMessage(const char *topic, const char *message);

const char *ssid = "COVID_19_5G_Hotspot_#421";
const char *password = "denkserw";
const char *mqtt_server = "192.168.100.3";

int status = WL_IDLE_STATUS; // the Wifi radio's status
WiFiEspClient espClient;
PubSubClient client(espClient);

void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, password);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  client.setServer(mqtt_server, 3011);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      while (true)
      {
        delay(500);
        publishMessage("my/topic", getTemperature());
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMessage(const char *topic, const char *message)
{
  int retryCount = 0;
  const int maxRetries = 3;

  while (retryCount < maxRetries)
  {
    if (client.connected())
    {
      if (client.publish(topic, message))
      {
        Serial.println("Message published successfully");
        break;
      }
      else
      {
        Serial.println("Message publishing failed, retrying...");
        retryCount++;
      }
    }
    else
    {
      Serial.println("Not connected to MQTT server, attempting to reconnect...");
      // Attempt to reconnect to the WiFi network here
      if (WiFi.begin(ssid, password) == WL_CONNECTED)
      {
        Serial.println("Reconnected to WiFi network");
        // Now attempt to reconnect to the MQTT server
        if (client.connect("mqtt_server"))
        {
          Serial.println("Reconnected to MQTT server");
        }
        else
        {
          Serial.println("Failed to reconnect to MQTT server, retrying...");
          retryCount++;
        }
      }
      else
      {
        Serial.println("Failed to reconnect to WiFi network, retrying...");
        retryCount++;
      }
    }
  }

  if (retryCount == maxRetries)
  {
    Serial.println("Failed to publish message after " + String(maxRetries) + " attempts");
  }
}
#include "activeMQ-client.service.h"

// Constructor
ActiveMQClientService::ActiveMQClientService() : mqttClient(wifiClient)
{
    mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                           { onMessageCallback(topic, payload, length); });
}

// Destructor
ActiveMQClientService::~ActiveMQClientService() {}

// Initialize the MQTT client
void ActiveMQClientService::initialize(const char *brokerAddress, int port, const char *clientId)
{
    mqttClient.setServer(brokerAddress, port);

    // Attempt to connect to the MQTT broker
    while (!mqttClient.connected())
    {
        Serial.println("Connecting to ActiveMQ broker...");
        if (mqttClient.connect(clientId))
        {
            Serial.println("Connected to ActiveMQ broker.");
        }
        else
        {
            Serial.print("Failed to connect. Retry in 5 seconds. Error code: ");
            Serial.println(mqttClient.state());
            delay(5000);
        }
    }
}

// Subscribe to a topic
void ActiveMQClientService::subscribe(const String &topic)
{
    if (mqttClient.connected())
    {
        mqttClient.subscribe(topic.c_str());
        Serial.println("Subscribed to topic: " + topic);
    }
    else
    {
        Serial.println("Cannot subscribe. MQTT not connected.");
    }
}

// Publish a JSON-formatted message to a topic
void ActiveMQClientService::publish(const String &topic, const JsonObject &message)
{
    if (!mqttClient.connected())
    {
        Serial.println("Cannot publish. MQTT not connected.");
        return;
    }

    char buffer[256];
    serializeJson(message, buffer);

    if (mqttClient.publish(topic.c_str(), buffer))
    {
        Serial.println("Message published to topic: " + topic);
    }
    else
    {
        Serial.println("Failed to publish message to topic: " + topic);
    }
}

// MQTT loop for processing
bool ActiveMQClientService::loop()
{
    return mqttClient.loop();
}

// Check if there are messages in the queue
bool ActiveMQClientService::hasMessage()
{
    return !messageQueue.empty();
}

// Retrieve the next message from the queue
String ActiveMQClientService::getNextMessage()
{
    if (hasMessage())
    {
        String message = messageQueue.front();
        messageQueue.pop();
        return message;
    }
    return "";
}

// Callback for handling incoming MQTT messages
void ActiveMQClientService::onMessageCallback(char *topic, byte *payload, unsigned int length)
{
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    // Process the message
    Serial.println("Received message on topic: " + String(topic));
    Serial.println("Message: " + message);

    // Queue the message for processing
    messageQueue.push(message);
}

// Helper to handle incoming messages
void ActiveMQClientService::handleIncomingMessage(const String &topic, const String &message)
{
    Serial.println("Processing message from topic: " + topic);
    Serial.println("Message: " + message);

    // Add custom processing logic here
}

#include "activeMQ-client.service.h"

// Initialize the static instance pointer
ActiveMQClientService *ActiveMQClientService::instance = nullptr;

// Constructor
ActiveMQClientService::ActiveMQClientService() : mqttClient(wifiClient)
{
    // Set this instance as the static instance
    instance = this;

    // Set the static callback for PubSubClient
    mqttClient.setCallback(onMessageCallback);
}

// Destructor
ActiveMQClientService::~ActiveMQClientService()
{
    instance = nullptr; // Clear the static instance
}

bool ActiveMQClientService::isConnected()
{
    return mqttClient.connected();
}

// Initialize the MQTT client
void ActiveMQClientService::initialize(const char *brokerAddress, int port, const char *clientId)
{
    // Set the static instance
    Serial.println("Initializing ActiveMQ client service. " + String(brokerAddress) + " " + String(port) + " " + String(clientId));
    mqttClient.setServer("192.168.100.102", 3011);

    // Attempt to connect to the MQTT broker
    while (!mqttClient.connected())
    {
        Serial.println("Connecting to ActiveMQ broker with clientId: " + String(clientId));
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
void ActiveMQClientService::publish(const String &topic, const JsonDocument &message)
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
MQTTMessage ActiveMQClientService::getNextMessage()
{
    if (hasMessage())
    {

        MQTTMessage message = messageQueue.front();
        messageQueue.pop();
        return message;
    }
    return {"", ""};
}

// Static MQTT callback for handling incoming messages
void ActiveMQClientService::onMessageCallback(char *topic, byte *payload, unsigned int length)
{
    if (instance)
    {
        String message = "";
        for (unsigned int i = 0; i < length; i++)
        {
            message += (char)payload[i];
        }

        // Use the instance to handle the message
        instance->handleIncomingMessage(String(topic), message);
    }
}

// Helper to handle incoming messages
void ActiveMQClientService::handleIncomingMessage(const String &topic, const String &message)
{
    Serial.println("Processing message from topic: " + topic);
    Serial.println("Message: " + message);
    MQTTMessage mqttMessage = {topic, message};

    // Queue the message for later processing
    messageQueue.push(mqttMessage);
}

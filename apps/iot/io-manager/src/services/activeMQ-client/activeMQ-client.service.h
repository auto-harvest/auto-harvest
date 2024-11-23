#ifndef ACTIVEMQ_CLIENT_SERVICE_H
#define ACTIVEMQ_CLIENT_SERVICE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <queue>
#include <WiFi.h>
#include <ArduinoJson.h>
struct MQTTMessage
{
    String topic;
    String message;
};
class ActiveMQClientService
{
public:
    ActiveMQClientService();
    ~ActiveMQClientService();

    void initialize(const char *brokerAddress, int port, const char *clientId);
    void subscribe(const String &topic);
    void publish(const String &topic, const JsonDocument &message);
    bool loop(); // Call this in the main loop for MQTT processing.

    bool hasMessage();            // Check if there are messages in the queue.
    MQTTMessage getNextMessage(); // Get the next message from the queue.
    bool isConnected();           // Check if the MQTT client is connected.
private:    
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    std::queue<MQTTMessage> messageQueue; // Queue to store incoming messages.

    // Static pointer to access the class instance
    static ActiveMQClientService *instance;

    // MQTT callback for handling incoming messages.
    static void onMessageCallback(char *topic, byte *payload, unsigned int length);

    // Helper to handle message processing.
    void handleIncomingMessage(const String &topic, const String &message);
};

#endif // ACTIVEMQ_CLIENT_SERVICE_H

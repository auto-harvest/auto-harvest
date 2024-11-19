#ifndef ACTIVEMQ_CLIENT_SERVICE_H
#define ACTIVEMQ_CLIENT_SERVICE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <queue>
#include <WiFi.h>
#include <ArduinoJson.h>

class ActiveMQClientService {
public:
    ActiveMQClientService();
    ~ActiveMQClientService();

    void initialize(const char* brokerAddress, int port, const char* clientId);
    void subscribe(const String& topic);
    void publish(const String& topic, const JsonObject& message);
    bool loop(); // Call this in the main loop for MQTT processing.

    bool hasMessage(); // Check if there are messages in the queue.
    String getNextMessage(); // Get the next message from the queue.

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    std::queue<String> messageQueue; // Queue to store incoming messages.

    // MQTT callback for handling incoming messages.
    static void onMessageCallback(char* topic, byte* payload, unsigned int length);

    // Helper to handle message processing.
    void handleIncomingMessage(const String& topic, const String& message);
};

#endif // ACTIVEMQ_CLIENT_SERVICE_H

#include "context/app.context.h"
auto dr = new DataCollector();

void setup()
{
    Serial.begin(115200);
    // initialize
    Wire.begin();
    AppContext &context = AppContext::getInstance();
    context.initialize();
    // context.initialize();

    // dr->initializeSensors();
}

boolean once = false;
void loop()
{
    AppContext &context = AppContext::getInstance();
    context.loop();
    // Serial.println("Looping...");
    // auto data = dr->collectData();
    // delay(1000);
}

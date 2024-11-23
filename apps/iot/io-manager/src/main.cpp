#include "context/app.context.h"

void setup()
{
    AppContext &context = AppContext::getInstance();
    context.initialize();
}

boolean once = false;
void loop()
{
    AppContext &context = AppContext::getInstance();
    context.loop();
}

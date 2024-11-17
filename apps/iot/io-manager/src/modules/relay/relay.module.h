#ifndef relay.module_H
#define relay.module_H

#include <Arduino.h>
extern const int relayPins[5]; // Declare relayPins as an extern variable
 
void initializeRelays();
void turnOnRelay(int relayPin);
void turnOffRelay(int relayPin);

#endif // relay.module_H
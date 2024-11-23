#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer
{
public:
    Timer(unsigned long interval);
    bool canRun();
    void reset();

private:
    unsigned long startTime;
    unsigned long interval;
};

#endif // TIMER_H
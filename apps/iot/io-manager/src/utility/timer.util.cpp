#include "timer.util.h"

Timer::Timer(unsigned long interval) : interval(interval)
{
    startTime = millis();
}

bool Timer::canRun()
{
    if (millis() - startTime >= interval)
    {
        startTime = millis();
        return true;
    }
    return false;
}

void Timer::reset()
{
    startTime = millis();
}
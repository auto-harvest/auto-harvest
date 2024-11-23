#include "waterFlow.sensor.h"

// Initialize static instance pointer
WaterFlowSensor *WaterFlowSensor::instance = nullptr;

WaterFlowSensor::WaterFlowSensor(uint8_t pin)
    : pin(pin), pulses(0), lastPulseTime(0), flowRate(0), lastFlowPinState(0)
{
    instance = this;
}

WaterFlowSensor::~WaterFlowSensor()
{
    instance = nullptr;
    useInterrupt(false); // Disable timer interrupt
}

void WaterFlowSensor::initialize()
{
    pinMode(pin, INPUT);                 // Set pin as input (no pull-up or pull-down)
    lastFlowPinState = digitalRead(pin); // Read initial state of the pin
    useInterrupt(true);                  // Enable the timer interrupt
}

std::map<std::string, double> WaterFlowSensor::readData()
{
    noInterrupts(); // Disable interrupts while accessing shared data
    float currentFlowRate = flowRate;
    uint16_t currentPulses = pulses;
    interrupts(); // Re-enable interrupts

    // Calculate liters based on pulse count (sensor-specific calculation)
    float liters = currentPulses;
    liters /= 7.5; // Frequency to Liters/min conversion (assuming plastic sensor)
    liters /= 60.0;

    std::map<std::string, double> data;
    data["flow-rate-hz"] = currentFlowRate;
    data["pulses"] = currentPulses;
    data["flow-rate-liters"] = liters;

    return data;
}

const char *WaterFlowSensor::getType()
{
    return "Flow Sensor";
}

const char *WaterFlowSensor::getSensorName()
{
    return "Liquid Flow Sensor";
}

void WaterFlowSensor::useInterrupt(boolean v)
{
    if (v)
    {
        // Timer0 is already used for millis(), we'll set the interrupt every 1 ms
        OCR0A = 0xAF;          // Set the timer to trigger every millisecond
        TIMSK0 |= _BV(OCIE0A); // Enable Timer0 Compare A interrupt
    }
    else
    {
        // Disable the interrupt
        TIMSK0 &= ~_BV(OCIE0A);
    }
}

// Timer interrupt called every 1ms
SIGNAL(TIMER0_COMPA_vect)
{
    uint8_t currentState = digitalRead(WaterFlowSensor::instance->pin);

    // If the state has changed (from LOW to HIGH or HIGH to LOW)
    if (currentState != WaterFlowSensor::instance->lastFlowPinState)
    {
        if (currentState == HIGH)
        {
            // Low to high transition
            WaterFlowSensor::instance->pulses++; // Increment pulse count
        }

        // Record the time between pulses (in milliseconds)
        unsigned long elapsedTime = millis() - WaterFlowSensor::instance->lastPulseTime;
        if (elapsedTime > 0)
        {
            // Calculate flow rate in Hz (pulses per second)
            WaterFlowSensor::instance->flowRate = 1000.0 / elapsedTime;
        }

        // Update last pulse time and state
        WaterFlowSensor::instance->lastPulseTime = millis();
        WaterFlowSensor::instance->lastFlowPinState = currentState;
    }

    // Increment the timer counter (time between pulses)
    WaterFlowSensor::instance->lastFlowRateTimer++;
}

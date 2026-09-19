#pragma once
#include <Bonezegei_DHT11.h>
#include "sensors.h"

// ============================================================================
// Temperature Sensor Class Definition
// DHT11 acquisition, timing, and measurement state
// ============================================================================

class TemperatureSensor
{
private:
    // Hardware Interface
    Bonezegei_DHT11 dht;

    // Sensor State
    float         temperature;
    unsigned long lastRead;
    bool          valid;

    // Timing
    static constexpr unsigned long READ_INTERVAL_MS = DHT_MS;

    //Statistics
    SignalStats stats;

public:
    // Lifecycle
    TemperatureSensor(uint8_t pin);

    //Data Acquisition
    void readTemperature();

    // Data Access
    float getCurrent() const;
    float getMin() const;
    float getMax() const;
    float getAvg() const;
};
#pragma once
#include <Arduino.h>

// ============================================================================
// Signal Statistics
// Stores and maintains real-time and session statistics for a telemetry signal.
// ============================================================================

class SignalStats
{
private:
    // Signal State
    float real_time = NAN;
    float minimum   = NAN;
    float maximum   = NAN;
    float avg       = NAN;

    // Sample Tracking
    uint32_t count = 0;

public:
    // Statistics Update
    void update(float newValue);

    // Data Access
    float    getRealTime() const;
    float    getMin()      const;
    float    getMax()      const;
    float    getAvg()      const;
    uint32_t getCount()    const;
    bool     hasData()     const;

    // Control
    void reset();
};
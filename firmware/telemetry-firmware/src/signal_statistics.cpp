#include <Arduino.h>
#include "sensors.h"
#include "signal_statistics.h"

// ============================================================================
// SignalStats — Method Definitions
// ============================================================================

void SignalStats::update(float newValue)
{
    // Ignore invalid measurements
    if (isnan(newValue))
        return;

    // Initialize statistics with the first valid measurement
    if (count == 0)
    {
        real_time = newValue;
        minimum   = newValue;
        maximum   = newValue;
        avg       = newValue;
        count     = 1;

        return;
    }

    // Update real-time value
    real_time = newValue;

    // Update minimum and maximum
    if (newValue < minimum)
        minimum = newValue;

    if (newValue > maximum)
        maximum = newValue;

    // Update sample count and running average
    count++;
    avg += (newValue - avg) / count;
}

// ----------------------------------------------------------------------------
// Statistics Reset
// ----------------------------------------------------------------------------

void SignalStats::reset()
{
    real_time = NAN;
    minimum   = NAN;
    maximum   = NAN;
    avg       = NAN;
    count     = 0;
}

// ----------------------------------------------------------------------------
// Data Access
// ----------------------------------------------------------------------------

float SignalStats::getRealTime() const
{
    return real_time;
}

float SignalStats::getMin() const
{
    return minimum;
}

float SignalStats::getMax() const
{
    return maximum;
}

float SignalStats::getAvg() const
{
    return avg;
}

uint32_t SignalStats::getCount() const
{
    return count;
}

bool SignalStats::hasData() const
{
    return count > 0;
}
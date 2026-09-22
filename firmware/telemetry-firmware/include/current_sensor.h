#pragma once
#include "sensors.h"
#include "lcd_screen.h"

// ============================================================================
// Current Sensor
// ACS712 current acquisition and zero-current calibration
// ============================================================================

class CurrentSensor
{
private:
    // Hardware Configuration
    uint8_t pin;

    // Sensor State
    float zeroCurrentVoltage = NAN;

    // Sensor Characteristics
    static constexpr float sensitivity = CURRENT_SENSITIVITY;  // ACS712-5A [V/A]

    //Statistics
    SignalStats stats;

public:
    // Constructor
    CurrentSensor(uint8_t pin);

    // Calibration
    void calibrate();

    // Data Acquisition
    void readCurrent();

    // Data Access
    float getZeroCurrentVoltage() const;
    float getCurrent() const;
    float getMin() const;
    float getMax() const;
    float getAvg() const;
};
#include <Arduino.h>
#include "pin_config.h"
#include "sensor_config.h"
#include "current_sensor.h"

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

CurrentSensor::CurrentSensor(uint8_t pin)
{
    this->pin = pin;
}

// ----------------------------------------------------------------------------
// Zero-Current Calibration
// ----------------------------------------------------------------------------

void CurrentSensor::calibrate()
{
    constexpr uint16_t CAL_SAMPLES = 200;

    unsigned long sum = 0;

    // Collect zero-current ADC samples
    for (uint16_t i = 0; i < CAL_SAMPLES; i++)
    {
        sum += analogRead(pin);
        delay(2);
    }

    // Calculate average ADC reading
    float averageADC = (float)sum / CAL_SAMPLES;

    // Convert ADC average to zero-current voltage
    zeroCurrentVoltage = averageADC * (ARDUINO_VOLTAGE_REF / 1023.0f);
}

// ----------------------------------------------------------------------------
// Current Acquisition
// ----------------------------------------------------------------------------

void CurrentSensor::readCurrent()
{
    // Read sensor ADC value
    uint16_t adcReading = analogRead(pin);

    // Convert ADC reading to sensor voltage
    float sensorVoltage = adcReading * (ARDUINO_VOLTAGE_REF / 1023.0f);

    // Convert sensor voltage to current
    float current = (sensorVoltage - zeroCurrentVoltage) / sensitivity;

    // Update signal statistics
    stats.update(current);
}

// ----------------------------------------------------------------------------
// Data Access
// ----------------------------------------------------------------------------

float CurrentSensor::getCurrent() const
{
    return stats.getRealTime();
}

float CurrentSensor::getMin() const
{
    return stats.getMin();
}

float CurrentSensor::getMax() const
{
    return stats.getMax();
}

float CurrentSensor::getAvg() const
{
    return stats.getAvg();
}
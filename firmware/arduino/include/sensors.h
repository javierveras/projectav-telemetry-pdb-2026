#pragma once
#include <Arduino.h>
#include "constants.h"
#include "bms.h"
#include "Bonezegei_DHT11.h"
#include "lcd_screen.h"

// ============================================================================
// Measurement & Sensor Constants
// ============================================================================

// ADC Reference
constexpr float ARDUINO_VOLTAGE_REF = 5.0f;

// Voltage Sensing
// Calibration constant to be experimentally determined in the laboratory.
constexpr float VOLTAGE_PROPORTIONALITY_CONST = 5.0f;

// Current Sensing — ACS712 5 A
constexpr float CURRENT_SENSITIVITY = 0.185f;  // V/A

// 4S Cell-Tap Voltage Divider — 47 kΩ / 10 kΩ
constexpr float R_TOP              = 47000.0f;  // Ω
constexpr float R_BOTTOM           = 10000.0f;  // Ω
constexpr float CELL_DIVIDER_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;

// ============================================================================
// Sensor
// Base class for all telemetry sensors.
// ============================================================================

class Sensor
{
protected:

    float realTime = NAN;
    float minimum  = NAN;
    float maximum  = NAN;
    float average  = NAN;
    String label;

    uint32_t sampleCount = 0;

    void updateStatistics(float newValue)
    {
        // Ignore invalid measurements
        if (isnan(newValue))
            return;

        // Initialize statistics with first valid measurement
        if (sampleCount == 0)
        {
            realTime   = newValue;
            minimum    = newValue;
            maximum    = newValue;
            average    = newValue;
            sampleCount = 1;

            return;
        }

        realTime = newValue;

        // Update extrema
        if (newValue < minimum)
            minimum = newValue;

        if (newValue > maximum)
            maximum = newValue;

        // Update running average
        sampleCount++;

        average += (newValue - average) / sampleCount;
    }


public:

    float getCurrent() const
    {
        return realTime;
    }

    float getMin() const
    {
        return minimum;
    }

    float getMax() const
    {
        return maximum;
    }

    float getAvg() const
    {
        return average;
    }

    uint32_t getSampleCount() const
    {
        return sampleCount;
    }

    bool hasData() const
    {
        return sampleCount > 0;
    }

    void resetStatistics()
    {
        realTime    = NAN;
        minimum     = NAN;
        maximum     = NAN;
        average     = NAN;
        sampleCount = 0;
    }

    virtual void read(){};

    void printToSerial() {
        Serial.print(label+"=");
        Serial.print(realTime, 2);
        Serial.print(",");
    };
};

class CurrentSensor : public Sensor
{
private:

    uint8_t pin;

    float zeroCurrentVoltage = NAN;

public:

    CurrentSensor(uint8_t pin, String Label)
        : pin(pin) {label=Label;}

    void calibrate()
    {
        Serial.println("calibrating..");
        constexpr uint16_t CAL_SAMPLES = 200;

        unsigned long sum = 0;

        // Collect zero-current ADC samples
        for (uint16_t i = 0; i < CAL_SAMPLES; i++)
        {
            sum += analogRead(pin);
            delay(2);
        }

        // Calculate average ADC reading
        float averageADC =
            sum / CAL_SAMPLES;

        Serial.print(averageADC);
        Serial.print("average ADC");

        // Convert ADC reading to sensor voltage
        zeroCurrentVoltage =
            averageADC *
            (ARDUINO_VOLTAGE_REF / 1023.0f);

    }

    void read() override
    {
        uint16_t adcReading = analogRead(pin);

        // Convert ADC value to sensor voltage
        float sensorVoltage =
            adcReading *
            (ARDUINO_VOLTAGE_REF / 1023.0f);

        // Convert sensor voltage to current
        float current =
            (sensorVoltage - zeroCurrentVoltage) /
            CURRENT_SENSITIVITY;

        updateStatistics(current);
    }

    float getZeroCurrentVoltage() const
    {
        return zeroCurrentVoltage;
    }
};


class TemperatureSensor : public Sensor
{
private:

    Bonezegei_DHT11 dht;

    unsigned long lastRead = 0;

    static constexpr unsigned long READ_INTERVAL_MS = DHT_MS;

public:

    TemperatureSensor(uint8_t pin, String Label)
        : dht(pin) {label=Label;}

    void read() override
    {
        const unsigned long now = millis();

        // Enforce minimum DHT11 sampling interval
        if (now - lastRead < READ_INTERVAL_MS)
            return;

        lastRead = now;

        // Request new DHT11 measurement
        if (!dht.getData())
            return;

        float temperature = dht.getTemperature();

        updateStatistics(temperature);
    }

};

CurrentSensor CURRENT_SENSORS[] = {
    { PIN::CURRENT_5V,  "05C" },
    { PIN::CURRENT_12V,  "12C" }
};

TemperatureSensor TEMPERATURE_SENSORS[] = {
    { PIN::BATTERY_TEMPERATURE_SENSOR, "BTP" }
};

Sensor* SENSORS[] = {
    &CURRENT_SENSORS[0],
    &CURRENT_SENSORS[1],
    &TEMPERATURE_SENSORS[0]
};

void CALIBRATE_CURRENT_SENSORS() {

  for (CurrentSensor& sensor : CURRENT_SENSORS) {
    sensor.calibrate();
  }

}
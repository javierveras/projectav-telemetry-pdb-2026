#pragma once

#include <Arduino.h>
#include "constants.h"
#include "bms.h"
#include "faults.h"
#include "Bonezegei_DHT11.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD_Screen(0x27, 20, 4);

// ============================================================================
// Measurement & Sensor Constants
// ============================================================================

// ADC Reference
constexpr float ARDUINO_VOLTAGE_REF = 5.0f;

// Voltage Sensing
constexpr float VOLTAGE_PROPORTIONALITY_CONST = 5.0f;

// Current Sensing — ACS712 5 A
constexpr float CURRENT_SENSITIVITY = 0.185f;  // V/A

// 4S Cell-Tap Voltage Divider — 47 kΩ / 10 kΩ
constexpr float R_TOP              = 47000.0f;
constexpr float R_BOTTOM           = 10000.0f;
constexpr float CELL_DIVIDER_RATIO = R_BOTTOM / (R_TOP + R_BOTTOM);


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

    uint8_t subsystem;
    uint8_t signal;

    float maxLimit;
    float minLimit;

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

    bool faultDetected = false;

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


    virtual void read() {}


    void printToSerial()
    {
        Serial.print(label);
        Serial.print("=");
        Serial.print(realTime, 2);
        Serial.print(",");
    }

    void checkThreshold(uint8_t faultIndex)
{
    if (isnan(realTime))
        return;
        

    // Above maximum
    if (realTime > maxLimit)
    {
        Fault fault(
            faultIndex,
            subsystem,
            signal,
            2,          // Severity
            2           // Type 2 = HIGH
        );

        faultDetected = true;
        fault.printFault();
    }

    // Below minimum
    else if (realTime < minLimit)
    {
        Fault fault(
            faultIndex,
            subsystem,
            signal,
            2,          // Severity
            1           // Type 1 = LOW
        );

        faultDetected = true;
        fault.printFault();
    }

    else{
        faultDetected = false;
    }
}
};


// ============================================================================
// Current Sensor
// ============================================================================

class CurrentSensor : public Sensor
{
private:

    uint8_t pin;

    float zeroCurrentVoltage = NAN;


public:

    CurrentSensor(
        uint8_t pin,
        uint8_t Subsystem,
        uint8_t Signal,
        String Label,
        float MaxLimit,
        float MinLimit
    )
        : pin(pin)
    {
        label     = Label;
        subsystem = Subsystem;
        signal    = Signal;
        maxLimit  = MaxLimit;
        minLimit  = MinLimit;
    }


    void calibrate()
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
        float averageADC =
            static_cast<float>(sum) / CAL_SAMPLES;

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


// ============================================================================
// Temperature Sensor
// ============================================================================

class TemperatureSensor : public Sensor
{
private:

    Bonezegei_DHT11 dht;

    unsigned long lastRead = 0;

    static constexpr unsigned long READ_INTERVAL_MS = DHT_MS;


public:

    TemperatureSensor(
        uint8_t pin,
        uint8_t Subsystem,
        uint8_t Signal,
        String Label
    )
        : dht(pin)
    {
        label     = Label;
        subsystem = Subsystem;
        signal    = Signal;
    }


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


// ============================================================================
// Voltage Sensor
// ============================================================================

class VoltageSensor : public Sensor
{
protected:

    uint8_t pin;


    float readVoltage()
    {
        uint16_t adcReading = analogRead(pin);

        // Convert ADC value to Arduino pin voltage
        float sensorVoltage =
            adcReading *
            (ARDUINO_VOLTAGE_REF / 1023.0f);

        // Recover voltage before voltage divider
        float voltage =
            sensorVoltage / CELL_DIVIDER_RATIO;

        return voltage;
    }


public:

    VoltageSensor(
        uint8_t pin,
        uint8_t Subsystem,
        uint8_t Signal,
        String Label,
        float MaxLimit,
        float MinLimit
    )
        : pin(pin)
    {
        label     = Label;
        subsystem = Subsystem;
        signal    = Signal;
        maxLimit  = MaxLimit;
        minLimit  = MinLimit;
    }


    void read() override
    {
        float voltage = readVoltage();

        updateStatistics(voltage);
    }
};


// ============================================================================
// Cell Sensor
//
// Battery cell taps are cumulative:
//
// Tap 1 = Cell 1
// Tap 2 = Cell 1 + Cell 2
// Tap 3 = Cell 1 + Cell 2 + Cell 3
// Tap 4 = Cell 1 + Cell 2 + Cell 3 + Cell 4
//
// Therefore:
//
// Cell 1 = Tap 1
// Cell 2 = Tap 2 - Tap 1
// Cell 3 = Tap 3 - Tap 2
// Cell 4 = Tap 4 - Tap 3
// ============================================================================

class CellSensor : public VoltageSensor
{
private:

    float tapVoltage = NAN;
    uint8_t ledPin; 

public:

    CellSensor(
        uint8_t pin,
        uint8_t Subsystem,
        uint8_t Signal,
        String Label,
        float MaxLimit,
        float MinLimit,
        uint8_t LedPin
    )
        : VoltageSensor(
            pin,
            Subsystem,
            Signal,
            Label,
            MaxLimit,
            MinLimit
        ),
        ledPin(LedPin)
    {}


    // Read cumulative voltage from battery tap
    void readTap()
    {
        tapVoltage = readVoltage();
    }


    float getTapVoltage() const
    {
        return tapVoltage;
    }


    // Convert cumulative tap voltage into individual cell voltage
    void updateCellVoltage(float previousTapVoltage)
    {
        if (isnan(tapVoltage))
            return;

        float cellVoltage =
            tapVoltage - previousTapVoltage;

        updateStatistics(cellVoltage);
    }

    void updateLED()
    {
        digitalWrite(ledPin, faultDetected ? HIGH : LOW);
    }

};


// ============================================================================
// Current Sensors
// ============================================================================

CurrentSensor CURRENT_SENSORS[] =
{
    {
        PIN::CURRENT_5V,
        SUBSYSTEM::POWER_DISTRIBUTION,
        4,
        "05C",
        3.0f,
        0.0f
    },

    {
        PIN::CURRENT_12V,
        SUBSYSTEM::POWER_DISTRIBUTION,
        2,
        "12C",
        6.0f,
        0.0f
    }
};


// ============================================================================
// Battery Cell Sensors
// ============================================================================

CellSensor CELL_VOLTAGE_SENSORS[] =
{
    {
        PIN::BATTERY_CELL_VOLTAGE[0],
        SUBSYSTEM::BATTERY,
        4,
        "BC1",
        3.75f,
        3.00f,
        PIN::LED_OUTPUT[0]
    },

    {
        PIN::BATTERY_CELL_VOLTAGE[1],
        SUBSYSTEM::BATTERY,
        5,
        "BC2",
        3.75f,
        3.00f,
        PIN::LED_OUTPUT[1]
    },

    {
        PIN::BATTERY_CELL_VOLTAGE[2],
        SUBSYSTEM::BATTERY,
        6,
        "BC3",
        3.75f,
        3.00f,
        PIN::LED_OUTPUT[2]
    },

    {
        PIN::BATTERY_CELL_VOLTAGE[3],
        SUBSYSTEM::BATTERY,
        7,
        "BC4",
        3.75f,
        3.00f,
        PIN::LED_OUTPUT[3]
    }
};


// ============================================================================
// Temperature Sensors
// ============================================================================

TemperatureSensor TEMPERATURE_SENSORS[] =
{
    {
        PIN::BATTERY_TEMPERATURE_SENSOR,
        SUBSYSTEM::BATTERY,
        3,
        "BTP"
    }
};


// ============================================================================
// Generic Sensor Array
// ============================================================================

Sensor* SENSORS[] =
{
    &CURRENT_SENSORS[0],
    &CURRENT_SENSORS[1]
};


// ============================================================================
// Current Sensor Calibration
// ============================================================================

void CALIBRATE_CURRENT_SENSORS()
{
    for (CurrentSensor& sensor : CURRENT_SENSORS)
    {
        sensor.calibrate();
    }
}


// ============================================================================
// Battery Cell Update
//
// IMPORTANT:
// All taps are measured FIRST.
//
// This minimizes error caused by measuring Tap 1, calculating Cell 1,
// waiting, then measuring Tap 2, etc.
// ============================================================================

void UPDATE_CELL_VOLTAGES()
{
    // --------------------------------------------------------
    // Step 1: Measure all cumulative battery taps
    // --------------------------------------------------------

    for (CellSensor& cell : CELL_VOLTAGE_SENSORS)
    {
        cell.readTap();
    }


    // --------------------------------------------------------
    // Step 2: Convert cumulative taps to individual cells
    // --------------------------------------------------------

    float previousTapVoltage = 0.0f;

    for (CellSensor& cell : CELL_VOLTAGE_SENSORS)
    {
        cell.updateCellVoltage(previousTapVoltage);

        previousTapVoltage =
            cell.getTapVoltage();
    }
}
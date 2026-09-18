#pragma once

#include <Arduino.h>

constexpr float ARDUINO_VOLTAGE_REF = 5.0f;
constexpr float VOLTAGE_PROPORTIONALITY_CONST = 5.0f; //Seperate from "V_REF", needs to be calculated in lab
constexpr float CURRENT_SENSITIVITY = 0.185f; //ACS712 @ 5A (V/A)
constexpr float ZERO_CURRENT_REF = ARDUINO_VOLTAGE_REF/2.0f;

// Voltage divider for 4S taps: 47 kΩ (top) / 10 kΩ (bottom)
constexpr float R_TOP    = 47000.0f;
constexpr float R_BOTTOM = 10000.0f;
constexpr float CELL_DIVIDER_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;

class TemperatureSensor {
    private:

    public:
    
};
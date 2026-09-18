#pragma once
#include <Arduino.h>

static constexpr float ARDUINO_VOLTAGE_REF = 5.0f;
static constexpr float VOLTAGE_PROPORTIONALITY_CONST = 5.0f; //Seperate from "V_REF"
static constexpr float CURRENT_SENSITIVITY = 0.185f; //ACS712 @ 5A (V/A)
static constexpr float ZERO_CURRENT_REF = ARDUINO_VOLTAGE_REF/2.0f;

// Voltage divider for 4S taps: 47 kΩ (top) / 10 kΩ (bottom)
static constexpr float R_TOP    = 47000.0f;
static constexpr float R_BOTTOM = 10000.0f;
static constexpr float CELL_DIVIDER_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;

float ADC_ToVoltage(uint16_t raw)
{
    return raw * ARDUINO_VOLTAGE_REF / 1023.0f;
}

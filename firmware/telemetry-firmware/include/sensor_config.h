#pragma once
#include <Arduino.h>

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
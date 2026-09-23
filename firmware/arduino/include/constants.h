#pragma once
#include <Arduino.h>

// Timing
constexpr unsigned long TELEMETRY_MS    = 250;     // system-wide refresh-rate
constexpr unsigned long DHT_MS          = 1200;    // DHT refresh rate, can't physically be less than 1000
constexpr uint16_t      ADC_SAMPLES     = 16;      // averages per reading 
constexpr uint16_t      BATCELL_SAMPLES = 32;      // higher due to cell noise

namespace PIN // Pin assignments
{
    constexpr uint8_t CURRENT_5V  = A0;
    constexpr uint8_t CURRENT_12V = A1;
    constexpr uint8_t VOLTAGE_5V  = A2;
    constexpr uint8_t VOLTAGE_12V = A3;
    constexpr uint8_t BATTERY_TEMPERATURE_SENSOR = 53;
    constexpr uint8_t BATTERY_CELL_VOLTAGE[4] = { A12, A13, A14, A15 };
}
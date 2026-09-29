#pragma once
#include <Arduino.h>

// Timing
constexpr unsigned long TELEMETRY_MS     = 250;     // system-wide refresh-rate
constexpr unsigned long DHT_MS           = 1200;    // DHT refresh rate, can't physically be less than 1000
constexpr unsigned long BUZZER_FREQUENCY = 500;     // Buzzer beeping frequency
constexpr uint16_t      ADC_SAMPLES      = 16;      // averages per reading 
constexpr uint16_t      BATCELL_SAMPLES  = 32;      // higher due to cell noise

namespace PIN // Pin assignments
{
    constexpr uint8_t CURRENT_5V  = A0;
    constexpr uint8_t CURRENT_12V = A1;
    constexpr uint8_t VOLTAGE_5V  = A2;
    constexpr uint8_t VOLTAGE_12V = A3;
    constexpr uint8_t BATTERY_TEMPERATURE_SENSOR = 53;
    constexpr uint8_t BATTERY_CELL_VOLTAGE[4] = { A12, A13, A14, A15 };
    constexpr uint8_t LED_OUTPUT[4] = { 2, 3, 4, 5 };
    constexpr uint8_t AMBIENT_TEMPERATURE_SENSOR = 54;
    constexpr uint8_t BUZZER = 12;
}

// System Fault Identifiers
namespace SUBSYSTEM {
    uint8_t POWER_DISTRIBUTION = 1;
    uint8_t BATTERY = 2;
    uint8_t MOBILITY = 3;
    uint8_t ENVIROMENT = 4;
}

namespace SEVERITY {
    uint8_t WARNING = 1;
    uint8_t CRITICAL = 2;
    uint8_t EMERGENCY = 3;
}

namespace TYPE {
    uint8_t HIGH_LIMIT = 1;
    uint8_t LOW_LIMIT = 2;
    uint8_t INVALID = 3;
    uint8_t SIGNAL_LOST = 4;
}
#pragma once

// ═══════════════════════════════════════════════════════════════════════════
//  Pin definitions  (edit here to remap – no other changes needed)
// ═══════════════════════════════════════════════════════════════════════════

constexpr uint8_t PIN_CURRENT_5V  = A0;
constexpr uint8_t PIN_CURRENT_12V = A1;
constexpr uint8_t PIN_VOLTAGE_5V  = A2;
constexpr uint8_t PIN_VOLTAGE_12V = A3;

// Single DHT11 – battery compartment temperature
static constexpr uint8_t TEMPERATURE_SENSOR_PIN = 53;

// 4S cell-tap analogue inputs  (A12 = V1 … A15 = V4)
static const uint8_t BATTERY_CELL_VOLTAGE_PINS[4] = { A12, A13, A14, A15 };

struct PowerChannel
{
    const char* name;
    uint8_t voltagePin;
    uint8_t currentPin;
};

constexpr PowerChannel POWER_CHANNELS[] =
{
    { "12V", PIN_VOLTAGE_12V, PIN_CURRENT_12V },
    { "5V",  PIN_VOLTAGE_5V, PIN_CURRENT_5V }
};




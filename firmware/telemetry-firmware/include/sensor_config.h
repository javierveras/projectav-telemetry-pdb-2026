#pragma once
#include "pin_config.h"

// ═══════════════════════════════════════════════════════════════════════════
//  Sensor / ADC parameters  (tune to match your hardware)
// ═══════════════════════════════════════════════════════════════════════════

static constexpr float VREF    = 5.0f;
static constexpr float ADC_MAX = 1023.0f;

// Voltage scaling: real_voltage = adc_voltage × V_SCALE
// Set to match your resistor-divider ratio on each voltage-sensor breakout.
static float V_SCALE[POWER_CHANNELS.size()] = { 5.0f, 5.0f };

// ACS712 sensitivity (V/A) – 5 A version = 0.185, 20 A = 0.100, 30 A = 0.066
static float I_SENS[POWER_CHANNELS.size()]  = { 0.185f, 0.185f };

// Zero-current ADC voltage (calibrated automatically at startup)
static float I_ZERO[POWER_CHANNELS.size()]  = { VREF / 2.0f, VREF / 2.0f };

// Voltage divider for 4S taps: 47 kΩ (top) / 10 kΩ (bottom)
static constexpr float R_TOP    = 47000.0f;
static constexpr float R_BOTTOM = 10000.0f;
static constexpr float DIV_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;
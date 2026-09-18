#pragma once
#include <Arduino.h>

constexpr unsigned long TELEMETRY_MS = 250;     // system-wide refresh-rate
constexpr unsigned long DHT_MS = 1200;          // DHT refresh rate, can't physically be less than 1000
constexpr uint16_t ADC_SAMPLES = 16;            // averages per reading 
constexpr uint16_t BATCELL_SAMPLES = 32;        // higher due to cell noise
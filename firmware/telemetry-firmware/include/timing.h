#pragma once
#include <Arduino.h>

static constexpr uint16_t      ADC_SAMPLES   = 16;     // averages per reading
static constexpr uint16_t      TAP_SAMPLES   = 50;
static constexpr unsigned long STATS_MS      = 5000;   // serial stats interval
static constexpr unsigned long TAP_PRINT_MS  = 500;    // cell-tap print interval
static constexpr unsigned long DHT_MS        = 2200;   // DHT minimum poll time
static constexpr unsigned long LCD_REFRESH   = 250;    // main loop delay
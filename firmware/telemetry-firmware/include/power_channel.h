#pragma once
#include "pin_config.h"
#include <Arduino.h>

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

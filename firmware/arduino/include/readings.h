#pragma once
#include "constants.h"
#include <Arduino.h>

struct PowerChannel
{
    String name;
    uint8_t voltagePin;
    uint8_t currentPin;
};

PowerChannel POWER_CHANNELS[] =
{
    { "12V", PIN::VOLTAGE_12V, PIN::CURRENT_12V },
    { "5V",  PIN::VOLTAGE_5V, PIN::CURRENT_5V },
    { "24V", PIN::VOLTAGE_12V, PIN::CURRENT_12V },
};

float CUMULATIVE_CELL_VOLTAGES[4]  = { NAN, NAN, NAN, NAN }; 

float INDIVIDUAL_CELL_VOLTAGES[4] = {
    CUMULATIVE_CELL_VOLTAGES[0],
    CUMULATIVE_CELL_VOLTAGES[1] - CUMULATIVE_CELL_VOLTAGES[0],
    CUMULATIVE_CELL_VOLTAGES[2] - CUMULATIVE_CELL_VOLTAGES[1],
    CUMULATIVE_CELL_VOLTAGES[3] - CUMULATIVE_CELL_VOLTAGES[2]
};   
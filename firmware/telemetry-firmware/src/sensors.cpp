#include <Arduino.h>
#include "pin_config.h"
#include "sensor_config.h"

float ADC_ToVoltage(uint16_t raw)
{
    return raw * ARDUINO_VOLTAGE_REF / 1023.0f;
}
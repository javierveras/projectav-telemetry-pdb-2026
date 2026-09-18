#pragma once
#include <Arduino.h>
#include "pin_config.h"
#include "sensor_config.h"
#include "timing.h"
#include "bms.h"
#include "temperature_sensor.h"
#include "current_sensor.h"
#include "signal_statistics.h"

TemperatureSensor batteryTemp(PIN_BATTERY_TEMPERATURE_SENSOR);
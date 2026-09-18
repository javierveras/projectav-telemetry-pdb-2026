#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Bonezegei_DHT11.h>
#include "pin_config.h"

LiquidCrystal_I2C LCD_Screen(0x27, 20, 4);
Bonezegei_DHT11 Temperature_Sensor(TEMPERATURE_SENSOR_PIN);
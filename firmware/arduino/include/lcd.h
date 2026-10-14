#pragma once

#include <Arduino.h>
#include "constants.h"
#include "bms.h"
#include "faults.h"
#include "Bonezegei_DHT11.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD_Screen(0x27, 20, 4);
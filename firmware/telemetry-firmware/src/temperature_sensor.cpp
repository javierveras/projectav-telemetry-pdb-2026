#include <Arduino.h>
#include "temperature_sensor.h"

TemperatureSensor::TemperatureSensor(uint8_t pin) : dht(pin) { // Constructor
    temperature = NAN;
    lastRead = 0;
    valid = false;
}
#include <Arduino.h>
#include "temperature_sensor.h"

TemperatureSensor::TemperatureSensor(uint8_t pin) : dht(pin) { // Constructor
    temperature = NAN;
    lastRead = 0;
    valid = false;
}

void TemperatureSensor::readTemperature()
{
    const unsigned long now = millis(); // Stores current time

    if(now - lastRead < READ_INTERVAL_MS) return;
    lastRead = now;

    if(dht.getData()){
        float temperature = dht.getTemperature();
        stats.update(temperature);
    }
}

// ----------------------------------------------------------------------------
// Data Access
// ----------------------------------------------------------------------------

float TemperatureSensor::getCurrent() const
{
    return stats.getRealTime();
}

float TemperatureSensor::getMin() const
{
    return stats.getMin();
}

float TemperatureSensor::getMax() const
{
    return stats.getMax();
}

float TemperatureSensor::getAvg() const
{
    return stats.getAvg();
}
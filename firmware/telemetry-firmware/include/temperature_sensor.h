#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Bonezegei_DHT11.h>
#include "pin_config.h"
#include "timing.h"

class TemperatureSensor {
    private:
    Bonezegei_DHT11 dht; //digital temperature and humidity sensor object
    float temperature;
    unsigned long lastRead;
    static constexpr unsigned long READ_INTERVAL_MS = DHT_MS;
    bool valid;

    public:
    TemperatureSensor(uint8_t pin);

    void update(){

    }

    float getTemperature() const;
    bool isValid() const;
};
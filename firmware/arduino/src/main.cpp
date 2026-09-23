#include <Arduino.h>
#include "sensors.h"
#include "readings.h"

void setup() {

  Serial.begin(115200);

for (CurrentSensor* sensor : ConverterCurrentSensors) {
    sensor->calibrate();
  }

};

void loop() {

  batteryTemp.readTemperature();

  ambientTemp.readTemperature();

}
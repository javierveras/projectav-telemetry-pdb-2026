#include <Arduino.h>
#include "sensors.h"
#include "bms.h"

unsigned long lastTelemetryTime;

void setup() {

  Serial.begin(115200);

  uint8_t FaultCount = 0;

  CALIBRATE_CURRENT_SENSORS();

  for(int pin : PIN::LED_OUTPUT) { // Set LED pins
    pinMode(pin, OUTPUT);
  }

  pinMode(PIN::BUZZER, OUTPUT);

  Serial.println("#START");

};

void loop() {

  for (VoltageSensor* sensor : CELL_VOLTAGE_SENSORS) {
    sensor->read();
  }

  // Transmit every 250 ms
  const unsigned long now = millis();

  if (now - lastTelemetryTime >= TELEMETRY_MS)
  {

  }

}
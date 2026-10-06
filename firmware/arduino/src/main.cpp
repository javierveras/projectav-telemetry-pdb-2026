#include <Arduino.h>
#include "sensors.h"
#include "bms.h"

unsigned long lastTelemetryTime;

void setup() {

  Serial.begin(115200);

  CALIBRATE_CURRENT_SENSORS();

  for(int pin : PIN::LED_OUTPUT) { // Set LED pins
    pinMode(pin, OUTPUT);
  }

  pinMode(PIN::BUZZER, OUTPUT);

  Serial.println("#START");

};

void loop() {

  UPDATE_CELL_VOLTAGES();

  // Transmit every 250 ms
  const unsigned long now = millis();

  if (now - lastTelemetryTime >= TELEMETRY_MS)
  {
    lastTelemetryTime = now;
    uint8_t faultIndex = 0;
  for (CellSensor& sensor : CELL_VOLTAGE_SENSORS)
  {
      sensor.printToSerial();
      sensor.checkThreshold(faultIndex);
      sensor.updateLED();
      faultIndex++;
  }
    Serial.println();
  }

}
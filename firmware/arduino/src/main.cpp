#include <Arduino.h>
#include "sensors.h"
#include "constants.h"
#include "bms.h"

unsigned long lastTelemetryTime;

void setup() {

  Serial.begin(115200);

  CALIBRATE_CURRENT_SENSORS();

  Serial.println("START_READING");

};

void loop() {

  for (Sensor* sensor : SENSORS) {
    sensor->read();
  }

  // Transmit every 250 ms
  const unsigned long now = millis();

  if (now - lastTelemetryTime >= TELEMETRY_MS)
  {
    lastTelemetryTime = now;

    for (Sensor* sensor : SENSORS)
    {
        sensor->printToSerial();
    }
    Serial.println();
  }

}
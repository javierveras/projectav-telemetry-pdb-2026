#include <Arduino.h>
#include "sensors.h"
#include "bms.h"

unsigned long lastTelemetryTime;
bool severeFault = false;

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
    severeFault = false;
  for (CellSensor& sensor : CELL_VOLTAGE_SENSORS)
  {
      sensor.printToSerial();
      sensor.checkThreshold(faultIndex);
      sensor.updateLED();
      if (sensor.severeFaultDetected)
        {
          severeFault = true;
        }
      faultIndex++;
  }
    Serial.println();
  }

  if (severeFault)
  {
      if ((millis() / 500) % 2 == 0)
      {
          tone(PIN::BUZZER, 2000);
      }
      else
      {
          noTone(PIN::BUZZER);
      }
  }
  else
  {
      noTone(PIN::BUZZER);
  }

}
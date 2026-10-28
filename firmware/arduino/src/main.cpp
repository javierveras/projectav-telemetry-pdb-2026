#include <Arduino.h>
#include "sensors.h"
#include "bms.h"
#include "lcd.h"

unsigned long lastTelemetryTime;
bool severeFault;

void setup() {

  Serial.begin(115200);
  Screen.begin();

  CALIBRATE_CURRENT_SENSORS();

  for(int pin : PIN::LED_OUTPUT) { // Set LED pins
    pinMode(pin, OUTPUT);
  }

  pinMode(PIN::BUZZER, OUTPUT); // Set Buzzer pin

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
    for (Sensor* sensor : ALL_SENSORS)
      {
        sensor->read();
        sensor->printToSerial();
        sensor->checkThreshold(faultIndex);
        sensor->updateLED();

        if (sensor->severeFaultDetected)
            severeFault = true;

        if (sensor->faultDetected)
            faultIndex++;
      }
      Serial.println();
  }
  buzzer.detectSevereFault(severeFault);
  Screen.update();
}
#include <Arduino.h>
#include "sensors.h"
#include "logger.h"
#include "bms.h"
#include "lcd.h"

unsigned long lastTelemetryTime;
bool severeFault;
uint16_t faults[12] = {};

void setup() {

  Serial.begin(115200);
  Screen.begin();
  DataLogger.begin();

  CALIBRATE_CURRENT_SENSORS();

  for(int pin : PIN::LED_OUTPUT) { // Set LED pins
    pinMode(pin, OUTPUT);
  }

  pinMode(PIN::BUZZER, OUTPUT); // Set Buzzer pin

  Serial.println("#START");

};

uint8_t logFrequencyCounter = 0;

void loop() {

  UPDATE_CELL_VOLTAGES();

  // Transmit every 250 ms
  const unsigned long now = millis();

  if (now - lastTelemetryTime >= TELEMETRY_MS)
  { 
    logFrequencyCounter++;
    Serial.print("TIME="+String(now));
    lastTelemetryTime = now;
    uint8_t faultIndex = 0;
    severeFault = false;
    String Data = String(now);
    for (Sensor* sensor : ALL_SENSORS)
    {
      sensor->read();
      Data += "," + String(sensor->getCurrent());
      sensor->printToSerial();
      sensor->checkThreshold(faultIndex);
      sensor->updateLED();
      if (sensor->severeFaultDetected)
          severeFault = true;

      if (sensor->faultDetected)
          faultIndex++;
    }
    DataLogger.log(Data);
    Serial.println();
    Screen.update();
    for (uint16_t& slot : faults)
    {
        slot = 0;
    }
    if(logFrequencyCounter >= 10)
    {
      DataLogger.save();
      logFrequencyCounter = 0;
    }
  }
  buzzer.detectSevereFault(severeFault);

}

//q bonita la vida
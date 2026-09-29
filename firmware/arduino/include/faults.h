#include <Arduino.h>
#include "sensors.h"
#include "bms.h"


class Fault {
    private:

    uint8_t index;
    uint8_t subsystem;
    uint8_t signal;
    uint8_t severity;
    uint8_t type;

    public:

    Fault(uint8_t Index, uint8_t Subsystem, uint8_t Signal, uint8_t Severity, uint8_t Type) {
        index = Index; subsystem = Subsystem; Signal = signal; severity = Severity; type = Type;
    }

    uint8_t getCode() {
        return (subsystem * 1000 + signal * 100 + severity * 10 + type);
    }

    void CheckSeverityForBuzzer() {

        bool buzzerState = false;
        unsigned long lastBuzzerToggle = 0;
        unsigned long now = millis();
        constexpr unsigned long BEEP_INTERVAL_MS = 500;

        if(severity == 3) {
            if (now - lastBuzzerToggle >= BEEP_INTERVAL_MS) {
                lastBuzzerToggle = now;
                buzzerState = !buzzerState;

                if (buzzerState)
                    tone(PIN::BUZZER, BUZZER_FREQUENCY);
                else
                    noTone(PIN::BUZZER);
            }
        }
    }

    void printFault() {

        Serial.print("f");
        Serial.print(index);
        Serial.print("=");
        Serial.print(getCode());

    }
};

Fault faults[20];
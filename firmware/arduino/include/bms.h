#pragma once
#include "constants.h"
#include <Arduino.h>

class Buzzer {
    private:
        uint8_t pin;

    public:
        Buzzer(int PIN) 
        {
            pin=PIN;
        }
    
    void detectSevereFault(bool severeFaultDetected) {
        if (severeFaultDetected)
        {
            if ((millis() / 100) % 2 == 0)
            {
                tone(pin, 2000);
            }
            else
            {
                noTone(pin);
            }
        }
        else
        {
            noTone(pin);
        }
    }

};

Buzzer buzzer(PIN::BUZZER);
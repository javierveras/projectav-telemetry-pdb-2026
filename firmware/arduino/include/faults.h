class Fault
{
private:
    uint8_t index;
    uint8_t subsystem;
    uint8_t signal;
    uint8_t severity;
    uint8_t type;

public:

    Fault(uint8_t Index,
          uint8_t Subsystem,
          uint8_t Signal,
          uint8_t Severity,
          uint8_t Type)
    {
        index     = Index;
        subsystem = Subsystem;
        signal    = Signal;     
        severity  = Severity;
        type      = Type;
    }

    uint16_t getCode() const     // FIXED: uint8_t can't hold 3402
    {
        return subsystem * 1000
             + signal * 100
             + severity * 10
             + type;
    }

    void printFault() const
    {
        Serial.print("f");
        Serial.print(index);
        Serial.print("=");
        Serial.print(getCode());
        Serial.print(",");
    }
};
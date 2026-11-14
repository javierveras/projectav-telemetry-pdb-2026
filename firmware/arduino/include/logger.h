
#pragma once
#include "constants.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class DataLog 
{
private:
    File DataFile;
    String currentFilename;

public:

    void begin()
    {
        if (!SD.begin(PIN::CHIP_SELECT))
        {
            Serial.println("SD initialization failed!");
            return;
        }

        // Find the next available session filename
        uint16_t session = 1;

        do
        {
            char filename[16];
            sprintf(filename, "/data%03u.csv", session);

            currentFilename = filename;
            session++;

        } while (SD.exists(currentFilename.c_str()));

        // Create the new session file
        DataFile = SD.open(currentFilename.c_str(), FILE_WRITE);

        if (DataFile)
        {
            DataFile.println(
                "TIME,B1V,B2V,B3V,B4V,05C,12C,FRC,FLC,BRC,BLC,05V,12V,BTP"
            );

            Serial.print("Logging to: ");
            Serial.println(currentFilename);
        }
        else
        {
            Serial.println("Failed to create session file!");
        }
    }

    void log(String data)
    {
        if (DataFile)
        {
            DataFile.println(data);
        }
    }

    void save()
    {
        if (DataFile)
        {
            DataFile.flush();
        }
    }
};

DataLog DataLogger;

#pragma once

#include <Arduino.h>
#include "constants.h"
#include "bms.h"
#include "faults.h"
#include "Bonezegei_DHT11.h"
#include <LiquidCrystal_I2C.h>

class LCD_Screen
{
private:
    LiquidCrystal_I2C lcd;
    unsigned long lastUpdate = 0;
    unsigned long lastPageChange = 0;
    float energyConsumed = 0.0f;
    float instantaneousPower = 0.0f;
    unsigned long lastEnergyUpdate = 0;
    uint8_t currentPage = 0;

public:
    LCD_Screen() : lcd(0x27, 20, 4) {}

    void begin()
    {
        lcd.init();
        lcd.backlight();
        lcd.clear();
    }

    void printRow(uint8_t row, const char* text)
    {
        lcd.setCursor(0, row);

        // Clear only the selected row
        lcd.print("                    ");

        lcd.setCursor(0, row);
        lcd.print(text);
    }

    void update()
    {
        unsigned long now = millis();

        // Change page every 3 seconds
        if (now - lastPageChange >= 3000)
        {
            lastPageChange = now;
            currentPage = (currentPage + 1) % 4;
            lcd.clear();
        }

        // Refresh readings every 250 ms
        if (now - lastUpdate < 250)
            return;

        lastUpdate = now;

        switch (currentPage)
        {
            case 0:
                showPower();
                break;

            case 1:
                showBattery();
                break;

            case 2:
                showMobility();
                break;
            case 3:
                showEnergy();
                break;
        }
    }

    void showPower();
    void showBattery();
    void showMobility();
    void showEnergy();
};


void LCD_Screen::showPower()
{
    lcd.setCursor(0, 0);
    lcd.print("POWER DISTRIBUTION  ");

    lcd.setCursor(0, 1);
    lcd.print("05V: ");
    lcd.print(VOLTAGE_SENSORS[0].getCurrent(), 2);
    lcd.print(" V      ");

    lcd.setCursor(0, 2);
    lcd.print("12V: ");
    lcd.print(VOLTAGE_SENSORS[1].getCurrent(), 2);
    lcd.print(" V      ");

    lcd.setCursor(0, 3);
    lcd.print("24V: ");
    lcd.print(VOLTAGE_SENSORS[2].getCurrent(), 2);
    lcd.print(" V      ");
}

void LCD_Screen::showBattery()
{
    lcd.setCursor(0, 0);
    lcd.print("BATTERY CELLS (V)   ");

    lcd.setCursor(0, 1);
    lcd.print("C1: ");
    lcd.print(CELL_VOLTAGE_SENSORS[0].getCurrent(), 2);
    lcd.setCursor(0, 2);
    lcd.print("C2: ");
    lcd.print(CELL_VOLTAGE_SENSORS[1].getCurrent(), 2);
    lcd.setCursor(9, 1);
    lcd.print("C3: ");
    lcd.print(CELL_VOLTAGE_SENSORS[2].getCurrent(), 2);
    lcd.setCursor(9, 2);
    lcd.print("C4: ");
    lcd.print(CELL_VOLTAGE_SENSORS[3].getCurrent(), 2);
    lcd.setCursor(0, 3);
    lcd.print("Total: ");
    float sum = CELL_VOLTAGE_SENSORS[0].getCurrent() +
                CELL_VOLTAGE_SENSORS[1].getCurrent() +
                CELL_VOLTAGE_SENSORS[2].getCurrent() +
                CELL_VOLTAGE_SENSORS[3].getCurrent();
    lcd.print(sum, 2);

}

void LCD_Screen::showMobility()
{
    lcd.setCursor(0, 0);
    lcd.print("WHEEL CURRENTS     ");

    for (int i = 0; i < 3; i++)
    {
        lcd.setCursor(0, i + 1);
        lcd.print("Wheel ");
        lcd.print(i + 1);
        lcd.print(": ");
        lcd.print(CURRENT_SENSORS[i + 2].getCurrent(), 2);
        lcd.print(" A      ");
    }
}

void LCD_Screen::showEnergy()
{
    unsigned long now = millis();

    // Battery voltage
    float batteryVoltage = 0.0f;

    for (CellSensor& sensor : CELL_VOLTAGE_SENSORS)
        batteryVoltage += sensor.getCurrent();

    // Regulated rail power (90% converter efficiency)
    float power5V =
        VOLTAGE_SENSORS[0].getCurrent() *
        CURRENT_SENSORS[0].getCurrent() / 0.90f;

    float power12V =
        VOLTAGE_SENSORS[1].getCurrent() *
        CURRENT_SENSORS[1].getCurrent() / 0.90f;

    // Wheel currents (assuming battery-side measurements)
    float wheelCurrent = 0.0f;

    for (int i = 2; i < 6; i++)
        wheelCurrent += CURRENT_SENSORS[i].getCurrent();

    instantaneousPower =
        power5V + power12V + batteryVoltage * wheelCurrent;

    // Integrate energy in Wh
    if (lastEnergyUpdate != 0)
    {
        float deltaHours =
            (now - lastEnergyUpdate) / 3600000.0f;

        energyConsumed += instantaneousPower * deltaHours;
    }

    lastEnergyUpdate = now;

    // LCD output
    lcd.setCursor(0, 0);
    lcd.print("ENERGY CONSUMPTION  ");

    lcd.setCursor(0, 1);
    lcd.print("Battery: ");
    lcd.print(batteryVoltage, 2);
    lcd.print(" V    ");

    lcd.setCursor(0, 2);
    lcd.print("Power: ");
    lcd.print(instantaneousPower, 2);
    lcd.print(" W      ");

    lcd.setCursor(0, 3);
    lcd.print("Session: ");
    lcd.print(energyConsumed, 2);
    lcd.print(" Wh     ");
}

LCD_Screen Screen;
#pragma once

constexpr bool  WARN_RAILS_ENABLE = false; // false-alarm before V_SCALE has been calibrated.  Set to true to enable.
constexpr float WARN_12V_MIN = 10.5f, WARN_12V_MAX = 13.5f;
constexpr float WARN_5V_MIN  = 4.5f,  WARN_5V_MAX  = 5.5f;

constexpr float WARN_TEMP_HIGH = 45.0f; // Battery compartment over-temperature (°C).  Note: DHT11 tops out at ~50 °C.

// Per-cell LiPo limits (V)
constexpr float WARN_CELL_UNDERVOLTAGE = 3.30f;   // undervoltage warning
constexpr float WARN_CELL_UNDERVOLTAGE_CRIT = 3.00f;   // critical undervoltage
constexpr float WARN_CELL_OVERVOLTAGE = 4.25f;   // overvoltage warning

constexpr float WARN_CELL_IMBAL = 0.20f; // Maximum spread between the highest and lowest of the 4 cells (V).
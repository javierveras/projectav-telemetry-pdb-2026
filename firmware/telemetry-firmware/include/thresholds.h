#pragma once

// Battery compartment over-temperature (°C).  Note: DHT11 tops out at ~50 °C.
static constexpr float WARN_TEMP_HIGH    = 45.0f;

// Per-cell LiPo limits (V)
static constexpr float WARN_CELL_UNDERVOLTAGE      = 3.30f;   // undervoltage warning
static constexpr float WARN_CELL_UNDERVOLTAGE_CRIT = 3.00f;   // critical undervoltage
static constexpr float WARN_CELL_OVERVOLTAGE      = 4.25f;   // overvoltage warning

// Cell imbalance: max spread between the highest and lowest of the 4 cells (V).
// Assumes all four taps are connected; a disconnected tap will read ~0 V and
// trip the critical-undervoltage check instead.
static constexpr float WARN_CELL_IMBAL   = 0.20f;

// Optional rail out-of-range check.  Disabled by default so it will not
// false-alarm before V_SCALE has been calibrated.  Set to true to enable.
static constexpr bool  WARN_RAILS_ENABLE = false;
static constexpr float WARN_12V_MIN = 10.5f, WARN_12V_MAX = 13.5f;
static constexpr float WARN_5V_MIN  = 4.5f,  WARN_5V_MAX  = 5.5f;

// How often an *already-active* warning set is re-printed on Serial (ms).
// State changes always print immediately regardless of this interval.
static constexpr unsigned long WARN_PRINT_MS = 3000;
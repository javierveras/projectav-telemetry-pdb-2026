/*
  File: mega_telemetry_plus_4s_tap_reader.ino
  Platform: Arduino Mega 2560

  ── Telemetry channels (LCD + Serial stats) ─────────────────────────────────
    ch0  12V rail  │ Voltage → A3   │ Current → A1
    ch1   5V rail  │ Voltage → A2   │ Current → A0

  ── Temperature (battery compartment, Serial + LCD) ─────────────────────────
    DHT11 DATA → Digital 53

  ── LCD (I²C, 20×4) ─────────────────────────────────────────────────────────
    SDA → 20  │  SCL → 21   │  I²C addr 0x27

  ── 4S LiPo cell taps (Serial only) — voltage divider 47 kΩ / 10 kΩ ────────
    A12 = 1S cumulative tap (V1)
    A13 = 2S cumulative tap (V2)
    A14 = 3S cumulative tap (V3)
    A15 = 4S cumulative tap (V4)

  ── Early-warning layer (LCD banner + Serial) ───────────────────────────────
    Flags battery over-temp, per-cell over/under-voltage (with a separate
    critical level), cell imbalance, and (optional) 12V/5V rail out-of-range.
    A flashing full-width LCD banner takes over the screen on any alarm and
    cycles through all active warnings; the serial monitor lists them in full.
    Thresholds live in the "Warning thresholds" block below.

  ── Calibration ─────────────────────────────────────────────────────────────
    On power-up the sketch measures I_ZERO for both current sensors with
    NO LOAD connected.  Do not connect loads until "Cal done" appears.

  ── Tuning constants ────────────────────────────────────────────────────────
    V_SCALE[]  – multiply measured ADC voltage to get real rail voltage.
                 Depends on your resistor-divider ratios.
                 Default: 5.0 (placeholder – adjust for your dividers).
    I_SENS[]   – ACS712 sensitivity (V/A).
                 0.185 → 5 A module,  0.100 → 20 A,  0.066 → 30 A.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Bonezegei_DHT11.h>
#include <math.h>

// ═══════════════════════════════════════════════════════════════════════════
//  Pin definitions  (edit here to remap – no other changes needed)
// ═══════════════════════════════════════════════════════════════════════════

static constexpr uint8_t N = 2;                  // number of power channels

// ch0 = 12V rail,  ch1 = 5V rail
static const uint8_t VP[N] = { A3, A2 };         // voltage-sensor analogue in
static const uint8_t CP[N] = { A1, A0 };         // current-sensor analogue in
static const char*   L[N]  = { "12V", "5V" };    // short labels for display

// Single DHT11 – battery compartment temperature
static constexpr uint8_t DHT_PIN = 53;

// 4S cell-tap analogue inputs
static const uint8_t TAP_PINS[4] = { A12, A13, A14, A15 };

// ═══════════════════════════════════════════════════════════════════════════
//  Sensor / ADC parameters  (tune to match your hardware)
// ═══════════════════════════════════════════════════════════════════════════

static constexpr float VREF    = 5.0f;
static constexpr float ADC_MAX = 1023.0f;

// Voltage scaling: real_voltage = adc_voltage × V_SCALE
// Set to match your resistor-divider ratio on each voltage-sensor breakout.
static float V_SCALE[N] = { 5.0f, 5.0f };

// ACS712 sensitivity (V/A) – 5 A version = 0.185, 20 A = 0.100, 30 A = 0.066
static float I_SENS[N]  = { 0.185f, 0.185f };

// Zero-current ADC voltage (calibrated automatically at startup)
static float I_ZERO[N]  = { VREF / 2.0f, VREF / 2.0f };

// Voltage divider for 4S taps: 47 kΩ (top) / 10 kΩ (bottom)
static constexpr float R_TOP    = 47000.0f;
static constexpr float R_BOTTOM = 10000.0f;
static constexpr float DIV_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;

// ═══════════════════════════════════════════════════════════════════════════
//  Warning thresholds  (early-warning layer — edit to taste)
//
//  Implements the manual's objective to "provide early warning for overheating
//  components or abnormal conditions" (System Introduction → Key Objectives)
//  and reinforces the battery-handling cautions in Appendix F.
// ═══════════════════════════════════════════════════════════════════════════

// Battery-compartment temperature (note: DHT11 only reads up to ~50 °C)
static constexpr float WARN_TEMP_C       = 45.0f;   // >= this  -> over-temp

// Per-cell LiPo limits (applied to each of the four derived cell voltages)
static constexpr float WARN_CELL_OV      = 4.25f;   // >= this  -> over-voltage
static constexpr float WARN_CELL_UV      = 3.30f;   // <= this  -> under-voltage (warn)
static constexpr float WARN_CELL_UV_CRIT = 3.00f;   // <= this  -> under-voltage (critical)
static constexpr float WARN_CELL_IMBAL   = 0.20f;   // (max-min) >= this -> imbalance

// Guard against false alarms when the pack is not connected / not being sensed:
// only evaluate cell warnings when the total tap voltage (V4) is at least this.
// A live 4S pack is always well above this even when deeply discharged.
static constexpr float PACK_PRESENT_MIN_V = 6.0f;

// Optional 12 V / 5 V rail range check.  OFF by default: with the placeholder
// V_SCALE the rail voltages are not yet meaningful, so enabling this before
// calibrating V_SCALE (Appendix C) would false-alarm.  Turn on only after the
// rail readings have been verified against a multimeter.
static constexpr bool  WARN_RAIL_ENABLE  = false;
static constexpr float WARN_RAIL_LO[N]   = { 10.5f, 4.5f };   // { 12V, 5V } lower
static constexpr float WARN_RAIL_HI[N]   = { 13.5f, 5.5f };   // { 12V, 5V } upper

// ═══════════════════════════════════════════════════════════════════════════
//  Timing
// ═══════════════════════════════════════════════════════════════════════════

static constexpr uint16_t      ADC_SAMPLES   = 16;     // averages per reading
static constexpr uint16_t      TAP_SAMPLES   = 50;
static constexpr unsigned long STATS_MS      = 5000;   // serial stats interval
static constexpr unsigned long TAP_PRINT_MS  = 500;    // cell-tap print interval
static constexpr unsigned long DHT_MS        = 2200;   // DHT minimum poll time
static constexpr unsigned long LCD_REFRESH   = 250;    // main loop delay
static constexpr unsigned long WARN_PRINT_MS = 1000;   // serial warning interval
static constexpr unsigned long BLINK_MS      = 500;    // LCD banner flash half-period

// ═══════════════════════════════════════════════════════════════════════════
//  Display format (decimal places)
// ═══════════════════════════════════════════════════════════════════════════

static constexpr uint8_t FMT_V = 1;
static constexpr uint8_t FMT_I = 2;
static constexpr uint8_t FMT_P = 1;
static constexpr uint8_t FMT_T = 1;

// ═══════════════════════════════════════════════════════════════════════════
//  Objects
// ═══════════════════════════════════════════════════════════════════════════

LiquidCrystal_I2C lcd(0x27, 20, 4);
Bonezegei_DHT11   dht(DHT_PIN);

// ═══════════════════════════════════════════════════════════════════════════
//  DHT state  (single sensor, battery compartment)
// ═══════════════════════════════════════════════════════════════════════════

static float         lastTemp   = NAN;
static unsigned long lastDhtMs  = 0;
static bool          lastDhtOk  = false;

// ═══════════════════════════════════════════════════════════════════════════
//  Latest 4S tap / cell readings
//  Refreshed on the tap timer and shared by the serial tap printer, the
//  warning evaluator, and the LCD banner.
// ═══════════════════════════════════════════════════════════════════════════

static float g_tapV[4]  = { NAN, NAN, NAN, NAN };   // cumulative V1..V4
static float g_cellV[4] = { NAN, NAN, NAN, NAN };   // individual C1..C4

// ═══════════════════════════════════════════════════════════════════════════
//  Statistics arrays  (min / rolling-mean / max)
// ═══════════════════════════════════════════════════════════════════════════

static float    vMin[N], vMax[N], vMean[N];
static float    iMin[N], iMax[N], iMean[N];
static float    pMin[N], pMax[N], pMean[N];
static float    tMin,    tMax,    tMean;      // single temp channel
static uint32_t vN[N],   iN[N],   pN[N],     tN;

// ═══════════════════════════════════════════════════════════════════════════
//  Warning model
// ═══════════════════════════════════════════════════════════════════════════

enum WarnCode : uint8_t {
  W_CELL_OV = 0,   // a cell is at/above the over-voltage limit
  W_CELL_UV_CRIT,  // a cell is at/below the critical under-voltage limit
  W_TEMP,          // battery compartment is at/above the over-temp limit
  W_CELL_UV,       // a cell is at/below the (warning) under-voltage limit
  W_IMBALANCE,     // spread between highest and lowest cell too large
  W_RAIL           // a 12V/5V rail is outside its allowed band
};

struct Warning {
  WarnCode code;
  uint8_t  idx;    // cell index (0-3) or rail index (0..N-1) where relevant
  float    value;  // offending measured value (or the spread, for imbalance)
};

// ═══════════════════════════════════════════════════════════════════════════
//  Utility helpers
// ═══════════════════════════════════════════════════════════════════════════

static inline float adcV(uint16_t raw) {
  return (raw * VREF) / ADC_MAX;
}

static uint16_t readAvg(uint8_t pin, uint16_t samples) {
  unsigned long s = 0;
  for (uint16_t i = 0; i < samples; i++) s += analogRead(pin);
  return (uint16_t)(s / samples);
}

// Welford online update: updates min, max, and rolling mean
static void upd(float x, float& mn, float& mx, float& mean, uint32_t& n) {
  if (isnan(x)) return;
  n++;
  mean += (x - mean) / (float)n;
  if (isnan(mn) || x < mn) mn = x;
  if (isnan(mx) || x > mx) mx = x;
}

static void statsInit() {
  for (uint8_t ch = 0; ch < N; ch++) {
    vMin[ch] = iMin[ch] = pMin[ch] = NAN;
    vMax[ch] = iMax[ch] = pMax[ch] = NAN;
    vMean[ch]= iMean[ch]= pMean[ch]= 0.0f;
    vN[ch]   = iN[ch]   = pN[ch]   = 0;
  }
  tMin = tMax = NAN;
  tMean = 0.0f;
  tN    = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Current-zero calibration  (called once at startup, no load attached)
// ═══════════════════════════════════════════════════════════════════════════

static void calibrateI0() {
  static constexpr uint16_t CAL_SAMPLES  = 200;
  static constexpr uint16_t CAL_DELAY_MS = 2;

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Cal I0: NO LOAD!");
  lcd.setCursor(0, 1); lcd.print("Remove all loads");
  Serial.println(F("\n== Current Zero Calibration (0 A – no load) =="));

  for (uint8_t ch = 0; ch < N; ch++) {
    unsigned long sum = 0;
    for (uint16_t k = 0; k < CAL_SAMPLES; k++) {
      sum += analogRead(CP[ch]);
      delay(CAL_DELAY_MS);
    }
    I_ZERO[ch] = adcV((uint16_t)((float)sum / CAL_SAMPLES + 0.5f));
    Serial.print(L[ch]);
    Serial.print(F(" I0 = "));
    Serial.print(I_ZERO[ch], 4);
    Serial.println(F(" V"));
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Cal done – ready");
  delay(800);
  lcd.clear();
}

// ═══════════════════════════════════════════════════════════════════════════
//  DHT update  (non-blocking, respects DHT_MS minimum poll interval)
// ═══════════════════════════════════════════════════════════════════════════

static void updateDhtTemp() {
  unsigned long now = millis();
  if (now - lastDhtMs < DHT_MS) return;
  lastDhtMs = now;

  bool ok = dht.getData();
  lastDhtOk = ok;
  if (ok) lastTemp = dht.getTemperature();
}

// ═══════════════════════════════════════════════════════════════════════════
//  LCD rendering — normal telemetry
//
//  Layout (20 × 4):
//    Row 0: "12V V:xx.x I:x.xx"
//    Row 1: "P:xx.x T:xx.xC"  (battery compartment temp)
//    Row 2: " 5V V:xx.x I:x.xx"
//    Row 3: "P:xx.x  [T shared]"
// ═══════════════════════════════════════════════════════════════════════════

static void lcdPrintCh(uint8_t ch, uint8_t row,
                        float v, float i, float p,
                        float t, bool showTemp) {
  // Line 1: label, voltage, current
  lcd.setCursor(0, row);
  lcd.print(F("                    "));
  lcd.setCursor(0, row);
  lcd.print(L[ch]);
  lcd.print(F(" V:"));  lcd.print(v, FMT_V);
  lcd.print(F(" I:"));  lcd.print(i, FMT_I);

  // Line 2: power and (optionally) temperature
  lcd.setCursor(0, row + 1);
  lcd.print(F("                    "));
  lcd.setCursor(0, row + 1);
  lcd.print(F("P:")); lcd.print(p, FMT_P);

  if (showTemp) {
    lcd.print(F(" T:"));
    if (lastDhtOk && !isnan(t)) {
      lcd.print(t, FMT_T);
      lcd.print((char)223);   // degree symbol
      lcd.print('C');
    } else {
      lcd.print(F("ERR"));
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  LCD rendering — flashing full-width warning banner
// ═══════════════════════════════════════════════════════════════════════════

static void lcdClearRow(uint8_t row) {
  lcd.setCursor(0, row);
  lcd.print(F("                    "));   // 20 spaces
  lcd.setCursor(0, row);
}

// Short headline (row 1) for one warning
static void lcdBannerHeadline(const Warning& w) {
  switch (w.code) {
    case W_CELL_OV:      lcd.print(F("CELL OVER-VOLTAGE"));  break;
    case W_CELL_UV_CRIT: lcd.print(F("CELL LOW-CRITICAL"));  break;
    case W_TEMP:         lcd.print(F("BATTERY OVER-TEMP"));  break;
    case W_CELL_UV:      lcd.print(F("CELL UNDER-VOLTAGE")); break;
    case W_IMBALANCE:    lcd.print(F("CELL IMBALANCE"));     break;
    case W_RAIL:         lcd.print(L[w.idx]); lcd.print(F(" RAIL FAULT")); break;
  }
}

// Detail line (row 2): offending value vs. limit, all kept <= 20 chars
static void lcdBannerDetail(const Warning& w) {
  switch (w.code) {
    case W_CELL_OV:
      lcd.print('C'); lcd.print(w.idx + 1); lcd.print(' ');
      lcd.print(w.value, 2); lcd.print(F("V >")); lcd.print(WARN_CELL_OV, 2);
      break;
    case W_CELL_UV_CRIT:
      lcd.print('C'); lcd.print(w.idx + 1); lcd.print(' ');
      lcd.print(w.value, 2); lcd.print(F("V <")); lcd.print(WARN_CELL_UV_CRIT, 2);
      break;
    case W_CELL_UV:
      lcd.print('C'); lcd.print(w.idx + 1); lcd.print(' ');
      lcd.print(w.value, 2); lcd.print(F("V <")); lcd.print(WARN_CELL_UV, 2);
      break;
    case W_TEMP:
      lcd.print(F("T ")); lcd.print(w.value, 1);
      lcd.print(F("C >")); lcd.print(WARN_TEMP_C, 1);
      break;
    case W_IMBALANCE:
      lcd.print(F("spread ")); lcd.print(w.value, 2); lcd.print('V');
      break;
    case W_RAIL:
      lcd.print(L[w.idx]); lcd.print('='); lcd.print(w.value, 1);
      lcd.print(F("V OUT"));
      break;
  }
}

// Draw the banner.  Flashes at ~1 Hz (BLINK_MS on / BLINK_MS off) and, when
// several warnings are active, advances to the next one each full blink cycle.
static void lcdRenderBanner(const Warning* w, uint8_t n) {
  bool on = ((millis() / BLINK_MS) % 2UL) == 0UL;

  if (!on) {                                   // flash-off phase: blank screen
    for (uint8_t r = 0; r < 4; r++) lcdClearRow(r);
    return;
  }

  uint8_t idx = (uint8_t)((millis() / (2UL * BLINK_MS)) % n);
  const Warning& cw = w[idx];

  lcdClearRow(0); lcd.print(F("!!!!! WARNING !!!!!"));
  lcdClearRow(1); lcdBannerHeadline(cw);
  lcdClearRow(2); lcdBannerDetail(cw);

  lcdClearRow(3);
  lcd.print('['); lcd.print(idx + 1); lcd.print('/'); lcd.print(n);
  lcd.print(F("] T:"));
  if (lastDhtOk && !isnan(lastTemp)) {
    lcd.print(lastTemp, FMT_T); lcd.print((char)223); lcd.print('C');
  } else {
    lcd.print(F("ERR"));
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Serial: telemetry statistics  (printed every STATS_MS ms)
// ═══════════════════════════════════════════════════════════════════════════

static void serialPrintTelemetryStatsIfDue() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < STATS_MS) return;
  last = now;

  auto trip = [](const __FlashStringHelper* k,
                 float mn, float mean, float mx,
                 const __FlashStringHelper* u, uint32_t n) {
    Serial.print(F("  ")); Serial.print(k); Serial.print(F(": "));
    if (!n) { Serial.println(F("no data")); return; }
    Serial.print(mn,   2); Serial.print(F(" / "));
    Serial.print(mean, 2); Serial.print(F(" / "));
    Serial.print(mx,   2); Serial.print(' ');
    Serial.println(u);
  };

  Serial.println(F("\n=== Telemetry Stats (min / avg / max) ==="));
  for (uint8_t ch = 0; ch < N; ch++) {
    Serial.println(L[ch]);
    trip(F("  V"), vMin[ch], vMean[ch], vMax[ch], F("V"), vN[ch]);
    trip(F("  I"), iMin[ch], iMean[ch], iMax[ch], F("A"), iN[ch]);
    trip(F("  P"), pMin[ch], pMean[ch], pMax[ch], F("W"), pN[ch]);
  }
  Serial.print(F("Battery compartment temp: "));
  if (!tN) { Serial.println(F("no data")); }
  else {
    Serial.print(tMin, 1); Serial.print(F(" / "));
    Serial.print(tMean,1); Serial.print(F(" / "));
    Serial.print(tMax, 1); Serial.println(F(" °C"));
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  4S cell-tap voltages
//    updateTaps()          – read taps, recover per-cell voltages into globals
//    updateTapsIfDue()     – refresh globals on the TAP_PRINT_MS timer
//    serialPrintTapsIfDue()– print globals on the TAP_PRINT_MS timer
// ═══════════════════════════════════════════════════════════════════════════

static float readTapVoltage(uint8_t pin) {
  uint16_t raw = readAvg(pin, TAP_SAMPLES);
  return adcV(raw) * DIV_RATIO;   // scale from divider node → actual tap V
}

static void updateTaps() {
  // Cumulative tap voltages (referenced to pack negative)
  g_tapV[0] = readTapVoltage(TAP_PINS[0]);
  g_tapV[1] = readTapVoltage(TAP_PINS[1]);
  g_tapV[2] = readTapVoltage(TAP_PINS[2]);
  g_tapV[3] = readTapVoltage(TAP_PINS[3]);

  // Individual cell voltages (differences between adjacent taps)
  g_cellV[0] = max(0.0f, g_tapV[0]);
  g_cellV[1] = max(0.0f, g_tapV[1] - g_tapV[0]);
  g_cellV[2] = max(0.0f, g_tapV[2] - g_tapV[1]);
  g_cellV[3] = max(0.0f, g_tapV[3] - g_tapV[2]);
}

static void updateTapsIfDue() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < TAP_PRINT_MS) return;
  last = now;
  updateTaps();
}

static void serialPrintTapsIfDue() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < TAP_PRINT_MS) return;
  last = now;

  Serial.print(F("Taps : V1=")); Serial.print(g_tapV[0], 3);
  Serial.print(F("  V2="));      Serial.print(g_tapV[1], 3);
  Serial.print(F("  V3="));      Serial.print(g_tapV[2], 3);
  Serial.print(F("  V4="));      Serial.print(g_tapV[3], 3);
  Serial.println(F("  V"));

  Serial.print(F("Cells: C1=")); Serial.print(g_cellV[0], 3);
  Serial.print(F("  C2="));      Serial.print(g_cellV[1], 3);
  Serial.print(F("  C3="));      Serial.print(g_cellV[2], 3);
  Serial.print(F("  C4="));      Serial.print(g_cellV[3], 3);
  Serial.println(F("  V"));
  Serial.println(F("--------------------------------------------------"));
}

// ═══════════════════════════════════════════════════════════════════════════
//  Warning evaluation
//  Pure function: reads the latest globals + this cycle's rail voltages and
//  fills `out` (up to maxOut) in priority order.  Returns the active count.
// ═══════════════════════════════════════════════════════════════════════════

static uint8_t evaluateWarnings(Warning* out, uint8_t maxOut, const float* railV) {
  uint8_t n = 0;
  auto push = [&](WarnCode c, uint8_t i, float val) {
    if (n < maxOut) { out[n].code = c; out[n].idx = i; out[n].value = val; n++; }
  };

  // Only trust the cell readings when the pack is actually connected/sensed.
  bool packPresent = (!isnan(g_tapV[3]) && g_tapV[3] >= PACK_PRESENT_MIN_V);

  // Gather cell facts (lowest, highest, worst over-voltage offender)
  float   cmin = NAN, cmax = NAN;
  uint8_t imin = 0,   imax = 0;
  bool    ovHit = false; uint8_t ovIdx = 0; float ovVal = 0.0f;
  if (packPresent) {
    for (uint8_t k = 0; k < 4; k++) {
      float c = g_cellV[k];
      if (isnan(c)) continue;
      if (isnan(cmin) || c < cmin) { cmin = c; imin = k; }
      if (isnan(cmax) || c > cmax) { cmax = c; imax = k; }
      if (c >= WARN_CELL_OV && (!ovHit || c > ovVal)) { ovHit = true; ovIdx = k; ovVal = c; }
    }
  }

  bool critLow   = packPresent && !isnan(cmin) && (cmin <= WARN_CELL_UV_CRIT);
  bool warnLow   = packPresent && !isnan(cmin) && !critLow && (cmin <= WARN_CELL_UV);
  bool imbalance = packPresent && !isnan(cmin) && !isnan(cmax) &&
                   ((cmax - cmin) >= WARN_CELL_IMBAL);
  bool tempHigh  = (lastDhtOk && !isnan(lastTemp) && lastTemp >= WARN_TEMP_C);

  // Push in priority order (most dangerous first)
  if (ovHit)     push(W_CELL_OV,      ovIdx, ovVal);
  if (critLow)   push(W_CELL_UV_CRIT, imin,  cmin);
  if (tempHigh)  push(W_TEMP,         0,     lastTemp);
  if (warnLow)   push(W_CELL_UV,      imin,  cmin);
  if (imbalance) push(W_IMBALANCE,    imax,  (cmax - cmin));

  if (WARN_RAIL_ENABLE && railV) {
    for (uint8_t ch = 0; ch < N; ch++) {
      if (isnan(railV[ch])) continue;
      if (railV[ch] < WARN_RAIL_LO[ch] || railV[ch] > WARN_RAIL_HI[ch])
        push(W_RAIL, ch, railV[ch]);
    }
  }

  return n;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Serial: warnings  (printed every WARN_PRINT_MS ms, only while active)
// ═══════════════════════════════════════════════════════════════════════════

static void serialPrintWarningLine(const Warning& w) {
  switch (w.code) {
    case W_CELL_OV:
      Serial.print(F("Cell "));  Serial.print(w.idx + 1);
      Serial.print(F(" OVER-VOLTAGE: "));  Serial.print(w.value, 3);
      Serial.print(F(" V (limit "));  Serial.print(WARN_CELL_OV, 2);
      Serial.println(F(" V)"));  break;
    case W_CELL_UV_CRIT:
      Serial.print(F("Cell "));  Serial.print(w.idx + 1);
      Serial.print(F(" CRITICAL LOW: "));  Serial.print(w.value, 3);
      Serial.print(F(" V (limit "));  Serial.print(WARN_CELL_UV_CRIT, 2);
      Serial.println(F(" V)"));  break;
    case W_TEMP:
      Serial.print(F("Battery compartment OVER-TEMP: "));  Serial.print(w.value, 1);
      Serial.print(F(" C (limit "));  Serial.print(WARN_TEMP_C, 1);
      Serial.println(F(" C)"));  break;
    case W_CELL_UV:
      Serial.print(F("Cell "));  Serial.print(w.idx + 1);
      Serial.print(F(" UNDER-VOLTAGE: "));  Serial.print(w.value, 3);
      Serial.print(F(" V (limit "));  Serial.print(WARN_CELL_UV, 2);
      Serial.println(F(" V)"));  break;
    case W_IMBALANCE:
      Serial.print(F("Cell IMBALANCE: spread "));  Serial.print(w.value, 3);
      Serial.print(F(" V (limit "));  Serial.print(WARN_CELL_IMBAL, 2);
      Serial.println(F(" V)"));  break;
    case W_RAIL:
      Serial.print(L[w.idx]);  Serial.print(F(" rail OUT OF RANGE: "));
      Serial.print(w.value, 2);  Serial.print(F(" V (allowed "));
      Serial.print(WARN_RAIL_LO[w.idx], 1);  Serial.print(F(" - "));
      Serial.print(WARN_RAIL_HI[w.idx], 1);  Serial.println(F(" V)"));  break;
  }
}

static void serialPrintWarningsIfDue(const float* railV) {
  static unsigned long last = 0;
  static uint8_t       prevCount = 0;
  unsigned long now = millis();
  if (now - last < WARN_PRINT_MS) return;
  last = now;

  Warning w[8];
  uint8_t n = evaluateWarnings(w, 8, railV);

  if (n == 0) {
    if (prevCount != 0)
      Serial.println(F(">> WARNINGS CLEARED - all monitored values nominal"));
    prevCount = 0;
    return;
  }

  Serial.print(F("\n*** WARNING"));
  if (n > 1) { Serial.print(F(" x")); Serial.print(n); }
  Serial.println(F(" ***"));
  for (uint8_t i = 0; i < n; i++) {
    Serial.print(F("  - "));
    serialPrintWarningLine(w[i]);
  }
  prevCount = n;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Arduino lifecycle
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  dht.begin();
  statsInit();

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F("Telemetry + 4S BMS"));
  lcd.setCursor(0, 1); lcd.print(F("12V & 5V  + LiPo"));
  lcd.setCursor(0, 2); lcd.print(F("DHT: pin 53"));
  delay(800);

  calibrateI0();
}

void loop() {
  // 1. Update temperature (non-blocking)
  updateDhtTemp();

  // 2. Refresh 4S tap / cell voltages into globals (on the tap timer)
  updateTapsIfDue();

  // 3. Read voltage, current, and power for each channel
  float v[N], i_ch[N], p[N];

  for (uint8_t ch = 0; ch < N; ch++) {
    float av = adcV(readAvg(VP[ch], ADC_SAMPLES));
    float ac = adcV(readAvg(CP[ch], ADC_SAMPLES));

    v[ch]    = fabsf(av * V_SCALE[ch]);
    i_ch[ch] = fabsf((ac - I_ZERO[ch]) / I_SENS[ch]);
    p[ch]    = fabsf(v[ch] * i_ch[ch]);

    upd(v[ch],    vMin[ch], vMax[ch], vMean[ch], vN[ch]);
    upd(i_ch[ch], iMin[ch], iMax[ch], iMean[ch], iN[ch]);
    upd(p[ch],    pMin[ch], pMax[ch], pMean[ch], pN[ch]);
  }

  // 4. Update temperature statistics
  upd(lastTemp, tMin, tMax, tMean, tN);

  // 5. Refresh LCD.
  //    A flashing full-width warning banner takes over the whole screen
  //    whenever any monitored value is out of range; otherwise the normal
  //    two-channel telemetry (12V rows 0-1, 5V rows 2-3) is shown.
  {
    Warning warns[8];
    uint8_t nWarn = evaluateWarnings(warns, 8, v);
    if (nWarn > 0) {
      lcdRenderBanner(warns, nWarn);
    } else {
      lcdPrintCh(0, 0, v[0], i_ch[0], p[0], lastTemp, true);
      lcdPrintCh(1, 2, v[1], i_ch[1], p[1], lastTemp, false);
    }
  }

  // 6. Serial output
  serialPrintTelemetryStatsIfDue();
  serialPrintTapsIfDue();
  serialPrintWarningsIfDue(v);

  delay(LCD_REFRESH);
}

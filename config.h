#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// FEATURE FLAGS
// ============================================================================
// The trip computer data (speed, distance, DTE, temperature, fuel) is sent
// to the triple meter via a proprietary Nissan serial protocol on pins 4/5.
// Set ENABLE_TRIP_SERIAL to 1 to attempt decoding it (experimental).
#define ENABLE_TRIP_SERIAL  0

// ============================================================================
// PIN ASSIGNMENTS
// ============================================================================
#define OIL_PIN       32    // Oil pressure sensor signal (ADC1)
#define VOLT_PIN      33    // Battery voltage via sensor module (ADC1)
#define SERIAL_RX_PIN 16    // RX from unified meter (pin 5 of M44) [optional]
#define SERIAL_TX_PIN 17    // TX to unified meter (pin 4 of M44) [optional]

// ============================================================================
// 350Z TRIPLE METER CONNECTOR M44 — VERIFIED FROM FSM (DI-30 to DI-52)
// ============================================================================
//
//  Pin  Wire Color   Signal                            Reference
//  ───  ──────────   ──────────────────────────────    ─────────────
//   1   B            Ground                            0V
//   2   R/W          Battery power (constant 12V)      Always batt V
//   3   G/Y          IGN ON/START power                Batt V (key ON)
//   4   P            TX serial → unified meter         Digital ~5V
//   5   L/B          RX serial ← unified meter         Digital ~5V
//   7   G/OR         Oil pressure sensor GROUND        0V
//   8   LG/R         Oil pressure sensor SIGNAL        ~1V(0psi) ~3V(72psi)
//   9   R/L          Oil pressure sensor +5V POWER     ~5V
//  12   R            Illumination signal               Variable
//
//  Pins 6, 10, 11 are not used.
//
// ── IMPORTANT NOTES FROM FSM ──
//  • The triple meter POWERS the oil sensor (+5V on pin 9, GND on pin 7).
//    Your circuit must provide this 5V or the sensor won't work.
//  • Trip computer data (speed, trip distance, DTE, outside temp, avg fuel,
//    avg speed, stopwatch, tire pressure) comes via serial protocol on pin 5.
//    There is NO direct VSS speed signal on this connector.
//  • The trip buttons (MODE/SET) are on the combination meter, not here.
//  • The voltmeter simply reads pin 3 (IGN power = battery voltage).
//
// ============================================================================
// WIRING: ESP32 to CONNECTOR M44
// ============================================================================
//
// ── POWER (from connector M44) ──
//
//   Pin 3 (G/Y, 12V IGN) ──→ LM2596 IN+
//   Pin 1 (B, Ground)     ──→ LM2596 IN-
//   LM2596 OUT+ (set 5V)  ──→ ESP32 Vin
//   LM2596 OUT-           ──→ ESP32 GND
//
// ── OIL PRESSURE SENSOR POWER (you must provide this!) ──
//
//   Pin 9 (R/L) ←── connect to 5V (from LM2596 OUT+ or ESP32 Vin)
//   Pin 7 (G/OR) ←── connect to GND
//
//   Without this, the oil pressure sensor gets no power and pin 8 = 0V.
//
// ── OIL PRESSURE SIGNAL (pin 8 → GPIO32) ──
//
//   Sensor outputs 0-5V. ESP32 ADC max is 3.3V. Use voltage divider:
//
//       Pin 8 (LG/R) ───[10KΩ]───┬───[10KΩ]─── GND
//                                 │
//                              GPIO32
//
//   FSM reference: ~1V at 0 PSI (engine off), ~3V at 500kPa (72.5 PSI)
//
// ── BATTERY VOLTAGE (pin 3 → voltage sensor → GPIO33) ──
//
//   Pin 3 (G/Y, 12V IGN) ──→ Voltage Sensor module (+)
//   Pin 1 (B, GND)        ──→ Voltage Sensor module (-)
//   Voltage Sensor S       ──→ GPIO33
//
// ── TRIP SERIAL (pins 4,5 → ESP32 Serial2) — OPTIONAL / EXPERIMENTAL ──
//
//   Pin 5 (L/B, RX data from unified meter) is ~5V logic.
//   Needs voltage divider for ESP32 3.3V:
//
//       Pin 5 (L/B) ───[10KΩ]───┬───[15KΩ]─── GND
//                                │
//                             GPIO16 (Serial2 RX)
//
//   Pin 4 (P, TX to unified meter):
//       GPIO17 (Serial2 TX) ──→ level shifter ──→ Pin 4
//       (only needed if you want to send data back)
//
// ============================================================================
//
//  COMPLETE CIRCUIT:
//
//    M44                    ESP32 / Modules
//   ┌────┐
//   │ 1  │─── GND ────────── ESP32 GND, LM2596 IN-, Volt Sensor (-)
//   │ 2  │─── 12V BATT ───── (optional: keep ESP32 on with key OFF)
//   │ 3  │─── 12V IGN ────── LM2596 IN+, Volt Sensor (+)
//   │ 4  │─── TX serial ──── (future: GPIO17 via level shifter)
//   │ 5  │─── RX serial ──── (future: GPIO16 via 10K/15K divider)
//   │ 7  │─── Sensor GND ─── GND (already connected)
//   │ 8  │─── Oil signal ─── [10K]──┬──[10K]──GND → GPIO32
//   │ 9  │─── Sensor 5V ──── 5V (from LM2596 or ESP32 Vin pin)
//   │ 12 │─── Illum ─────── (optional: dim OLEDs with dash lights)
//   └────┘
//
//                LM2596 (set to 5V)
//   12V IGN ──→ IN+          OUT+ ──→ ESP32 Vin + Pin 9 (sensor 5V)
//   GND ──────→ IN-          OUT- ──→ ESP32 GND
//
//                Voltage Sensor (VCC<25V)
//   12V IGN ──→ (+)          S ──→ GPIO33
//   GND ──────→ (-)          VCC ──→ ESP32 3.3V
//                             GND ──→ ESP32 GND
//
//                TCA9548A
//   ESP32 SDA (21) ──→ SDA     SD0/SC0 ──→ OLED 0 (Oil Pressure)
//   ESP32 SCL (22) ──→ SCL     SD1/SC1 ──→ OLED 1 (Battery Voltage)
//   ESP32 3.3V ──────→ VIN     SD2/SC2 ──→ OLED 2 (Trip/Info)
//   ESP32 GND ───────→ GND
//
// ============================================================================

// ── OIL PRESSURE CALIBRATION (from FSM reference values) ──
// Sensor output through 10K/10K divider:
//   ~1V sensor = 0 PSI → 0.5V at ESP32
//   ~3V sensor = 500kPa (72.5 PSI) → 1.5V at ESP32
//   ~4.5V sensor = max → 2.25V at ESP32
#define OIL_DIVIDER     0.5     // 10K/(10K+10K) voltage divider ratio
#define OIL_V_MIN       1.0     // Sensor voltage at 0 PSI (FSM: ~1V engine off)
#define OIL_V_MAX       4.5     // Sensor voltage at max PSI
#define OIL_PSI_MAX     120.0   // Full scale PSI
// FSM data points: 1V=0psi, 3V=72.5psi → slope ≈ 36.25 PSI/V from 1V

// ── BATTERY VOLTAGE CALIBRATION ──
#define VOLT_FACTOR   4.648     // Adjust to match multimeter

// ── ADC ──
#define ADC_SAMPLES   16

// ── TIMING ──
#define DISPLAY_INTERVAL_MS  100
#define SAVE_INTERVAL_MS     30000

#endif

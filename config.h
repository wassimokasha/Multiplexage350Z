#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// FEATURE FLAGS — set to 0 to disable features you haven't wired yet
// ============================================================================
#define ENABLE_VSS      0   // Set to 1 when you wire the VSS speed signal
#define ENABLE_BUTTON   0   // Set to 1 when you wire the trip button

// ============================================================================
// PIN ASSIGNMENTS
// ============================================================================
#define OIL_PIN       32    // Oil pressure sender (ADC1)
#define VOLT_PIN      33    // Battery voltage sensor (ADC1)
#define VSS_PIN       34    // Vehicle speed signal (input-only, interrupt)
#define BUTTON_PIN    35    // Trip/clock button (input-only)

// ============================================================================
// WIRING: ESP32 to 350Z CENTER CONSOLE GAUGE CONNECTOR
// ============================================================================
//
// The 350Z center gauge pod connector carries these signals:
//
//  CONNECTOR PIN          WIRE TO ESP32
//  ─────────────────────  ──────────────────────────────────────────────
//  12V IGN (switched)     Vin of your voltage regulator (powers ESP32)
//                         AND through your voltage sensor module → GPIO33
//
//  Ground                 ESP32 GND + voltage regulator GND
//
//  Oil pressure sender    See circuit below → GPIO32
//  signal wire
//
//  VSS (speed pulses      Voltage divider (see below) → GPIO34
//  for clock/trip unit)   [OPTIONAL — set ENABLE_VSS to 1]
//
//  Clock button           10K pull-up to 3.3V → GPIO35
//  (press to cycle)       [OPTIONAL — set ENABLE_BUTTON to 1]
//
// ── OIL PRESSURE SENDER CIRCUIT ──
//       3.3V ───[100Ω]───┬─── Oil sender signal pin (on connector)
//                         │
//                      GPIO32
//
// ── VSS (VEHICLE SPEED SIGNAL) CIRCUIT ──
//       VSS pin ───[10KΩ]───┬───[4.7KΩ]─── GND
//                            │
//                         GPIO34
//
// ── BUTTON CIRCUIT ──
//       3.3V ───[10KΩ]───┬─── GPIO35
//                         │
//                      Button ─── GND
//
// ============================================================================

// ── CALIBRATION CONSTANTS ──

// Oil pressure: pull-up resistor value (ohms)
#define OIL_R_PULLUP  100.0
// Sender resistance range (Nissan VQ35DE sender, approximate)
#define OIL_R_MIN     10.0    // Resistance at 0 PSI
#define OIL_R_MAX     184.0   // Resistance at max PSI
#define OIL_PSI_MAX   120.0   // Full scale PSI

// Battery voltage divider factor (your voltage sensor module calibration)
// Adjust until the reading matches a multimeter on the battery.
#define VOLT_FACTOR   4.648

// VSS: pulses per kilometer (350Z stock ~4000 pulses/mile ≈ 2485/km)
#define VSS_PULSES_PER_KM  2485.0

// ADC oversampling (averages this many reads for noise reduction)
#define ADC_SAMPLES   16

// Display refresh interval (ms) — ~10 Hz
#define DISPLAY_INTERVAL_MS  100

// Flash save interval (ms) — every 30 seconds
#define SAVE_INTERVAL_MS     30000

#endif

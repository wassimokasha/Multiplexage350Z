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
#define OIL_PIN       32    // Oil pressure signal (ADC1)
#define VOLT_PIN      33    // Battery voltage sensor module (ADC1)
#define VSS_PIN       34    // Vehicle speed signal (input-only, interrupt)
#define BUTTON_PIN    35    // Trip/clock button (input-only)

// ============================================================================
// 350Z TRIPLE METER CONNECTOR
// ============================================================================
//
// The 3 center console gauges (oil, volt, clock) are a single unit called
// the "Triple Meter Assembly" (Nissan P/N: 24845-CD000).
// It connects via ONE white connector on the back of the unit.
//
// ── CONFIRMED PINS ──
//   Pin 8:  Oil pressure signal  |  Light Green/Red wire  |  0-5V analog
//   Pin ?:  12V IGN (switched)   |  (probe with multimeter, key ON)
//   Pin ?:  Ground               |  (continuity to chassis)
//   Pin ?:  12V BATT (constant)  |  (12V even with key OFF, for clock memory)
//   Pin ?:  VSS speed signal     |  (pulses when wheels spin)
//   Pin ?:  Illumination +/-     |  (dims with dash lights)
//   Pin ?:  Clock button         |  (from the trip/clock button)
//
// To identify unknown pins: use a multimeter on the car-side connector
// with key ON. Or download the FSM (Factory Service Manual) section "DI"
// from nicoclub.com/FSM/350Z/ — it has the full terminal layout.
//
// ── OIL PRESSURE SENSOR (25070-CD00A) ──
// This is a 3-wire 0-5V analog sensor on the engine block.
// The ECU provides +5V reference; pin 8 carries the 0-5V output signal.
// Sensor stays powered by ECU even with triple meter disconnected.
//
//   Sensor connector (at engine):
//     Red   = +5V reference (from ECU)
//     Green = Signal output (0-5V proportional to pressure)
//     Black = Ground
//
//   FSM pressure specs:
//     Idle:      ~14 PSI   (~0.8V)
//     2000 RPM:  ~43 PSI   (~2.1V)
//     6000 RPM:  ~57 PSI   (~2.7V)
//
// ============================================================================
// WIRING: ESP32 to 350Z CONNECTOR
// ============================================================================
//
// ── POWER ──
//   12V IGN pin ──→ LM2596 IN+ ──→ set to 5V ──→ ESP32 Vin
//   Ground pin  ──→ LM2596 IN- ──→ ESP32 GND
//
// ── BATTERY VOLTAGE (GPIO33) ──
//   12V IGN pin ──→ Voltage Sensor module (+) input
//   Ground pin  ──→ Voltage Sensor module (-) input
//   Voltage Sensor S output ──→ GPIO33
//
// ── OIL PRESSURE (GPIO32) ──
//   Pin 8 signal is 0-5V. ESP32 ADC max is 3.3V → MUST step down!
//   Use two 10KΩ resistors as a voltage divider:
//
//       Pin 8 (oil signal) ───[10KΩ]───┬───[10KΩ]─── GND
//                                       │
//                                    GPIO32
//
//   This halves the voltage: 0-5V → 0-2.5V (safe for ESP32)
//
// ── VSS (GPIO34) — OPTIONAL ──
//   VSS is typically 0-12V square wave. Step down:
//
//       VSS pin ───[10KΩ]───┬───[4.7KΩ]─── GND
//                            │
//                         GPIO34
//
// ── BUTTON (GPIO35) — OPTIONAL ──
//       Clock button pin ───→ GPIO35
//       3.3V ───[10KΩ]───→ GPIO35  (external pull-up required)
//
// ============================================================================

// ── OIL PRESSURE CALIBRATION ──
// The sensor outputs 0.5V at 0 PSI and ~4.5V at max PSI (typical Nissan).
// After the 10K/10K voltage divider, ESP32 sees half: 0.25V - 2.25V.
// Divider ratio = 0.5 (bottom resistor / total resistance)
#define OIL_DIVIDER     0.5     // 10K/(10K+10K) voltage divider ratio
#define OIL_V_MIN       0.5     // Sensor output voltage at 0 PSI
#define OIL_V_MAX       4.5     // Sensor output voltage at max PSI
#define OIL_PSI_MAX     120.0   // Full scale PSI

// ── BATTERY VOLTAGE CALIBRATION ──
// Your voltage sensor module's calibration factor.
// Adjust until the reading matches a multimeter on the battery.
#define VOLT_FACTOR   4.648

// ── VSS CALIBRATION ──
// 350Z stock: ~4000 pulses/mile ≈ 2485 pulses/km
#define VSS_PULSES_PER_KM  2485.0

// ── ADC ──
#define ADC_SAMPLES   16    // Oversampling for noise reduction

// ── TIMING ──
#define DISPLAY_INTERVAL_MS  100    // ~10 Hz refresh
#define SAVE_INTERVAL_MS     30000  // Save trip to flash every 30s

#endif

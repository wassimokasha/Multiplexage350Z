#include "sensors.h"
#include <Preferences.h>

// ============================================================================
// GLOBALS
// ============================================================================

float oilPSI     = 0;
float battVolts   = 0;

int clockH = 12, clockM = 0, clockS = 0;

// Serial trip data from unified meter (experimental)
float serialSpeed       = 0;
float serialOutsideTemp = 0;
bool  serialDataValid   = false;

static Preferences prefs;
static unsigned long lastClockTick = 0;
static unsigned long lastSave = 0;

// ============================================================================
// ADC HELPERS
// ============================================================================

static float readADC(int pin) {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) sum += analogRead(pin);
  return (float)sum / ADC_SAMPLES;
}

static float oilADCtoPSI(float adc) {
  // ADC reads the divided voltage. Recover the actual sensor voltage.
  float v_adc = adc * (3.3 / 4095.0);
  float v_sensor = v_adc / OIL_DIVIDER;    // undo 10K/10K divider

  // FSM reference: ~1V = 0 PSI, ~3V = 500kPa (72.5 PSI), ~4.5V = max
  // Linear mapping from OIL_V_MIN to OIL_V_MAX
  float psi = (v_sensor - OIL_V_MIN) / (OIL_V_MAX - OIL_V_MIN) * OIL_PSI_MAX;
  return constrain(psi, 0, OIL_PSI_MAX);
}

// ============================================================================
// INIT
// ============================================================================

void sensorsInit() {
  pinMode(OIL_PIN, INPUT);
  pinMode(VOLT_PIN, INPUT);

#if ENABLE_TRIP_SERIAL
  // Pin 5 of M44 (RX from unified meter) → GPIO16 via voltage divider
  // Try common automotive baud rates. 19200 is typical for Nissan serial.
  // If no data, try: 9600, 31250, 38400, 57600
  Serial2.begin(19200, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
#endif

  lastClockTick = millis();
  lastSave = millis();
}

// ============================================================================
// FLASH PERSISTENCE
// ============================================================================

void loadFromFlash() {
  prefs.begin("trip350z", true);
  clockH = prefs.getInt("cH", 12);
  clockM = prefs.getInt("cM", 0);
  prefs.end();
}

void saveToFlash() {
  unsigned long now = millis();
  if (now - lastSave < SAVE_INTERVAL_MS) return;
  lastSave = now;

  prefs.begin("trip350z", false);
  prefs.putInt("cH", clockH);
  prefs.putInt("cM", clockM);
  prefs.end();
}

// ============================================================================
// SENSOR READS
// ============================================================================

void readOilPressure() {
  float psi = oilADCtoPSI(readADC(OIL_PIN));
  oilPSI = oilPSI * 0.7 + psi * 0.3;  // exponential smoothing
}

void readBatteryVoltage() {
  float raw = readADC(VOLT_PIN);
  float v = (raw / 4095.0) * 3.3 * VOLT_FACTOR;
  battVolts = battVolts * 0.7 + v * 0.3;
}

void updateClock() {
  unsigned long now = millis();
  unsigned long dt = now - lastClockTick;
  if (dt < 1000) return;
  lastClockTick = now;

  clockS += dt / 1000;
  while (clockS >= 60) { clockS -= 60; clockM++; }
  while (clockM >= 60) { clockM -= 60; clockH++; }
  while (clockH >= 24) { clockH -= 24; }
}

// ============================================================================
// TRIP SERIAL DECODER (EXPERIMENTAL)
// ============================================================================
// The unified meter & A/C amp sends trip computer data to the triple meter
// via a proprietary Nissan serial protocol on pin 5 (L/B wire).
// This includes: vehicle speed, outside temp, DTE, avg fuel consumption,
// avg speed, trip distance, trip time, tire pressure, etc.
//
// The protocol is undocumented. This function logs raw bytes to Serial
// for reverse-engineering. Once decoded, it can populate serialSpeed,
// serialOutsideTemp, etc.

void processSerialData() {
#if ENABLE_TRIP_SERIAL
  static uint8_t buf[64];
  static int bufIdx = 0;

  while (Serial2.available()) {
    uint8_t b = Serial2.read();

    // Log raw bytes for reverse-engineering
    Serial.printf("%02X ", b);
    bufIdx++;
    if (bufIdx >= 32) {
      Serial.println();
      bufIdx = 0;
    }

    // TODO: Once the protocol is understood, parse frames here
    // and populate serialSpeed, serialOutsideTemp, serialDataValid, etc.
    //
    // Typical automotive serial frames have:
    //   [header/sync byte] [command/address] [data bytes] [checksum]
    //
    // Try watching the raw output while driving at known speeds
    // to correlate byte patterns with actual values.
  }
#endif
}

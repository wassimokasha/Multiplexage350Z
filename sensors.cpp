#include "sensors.h"
#include <Preferences.h>

// ============================================================================
// GLOBALS
// ============================================================================

float oilPSI    = 0;
float battVolts  = 0;
float speedKmh   = 0;
float tripA_km   = 0;
float tripB_km   = 0;

int clockH = 12, clockM = 0, clockS = 0;
int displayMode = 0;

static Preferences prefs;

// VSS
#if ENABLE_VSS
static volatile unsigned long vssCounter = 0;
static unsigned long lastSpeedCalc = 0;

void IRAM_ATTR vssISR() {
  vssCounter++;
}
#endif

// Button
#if ENABLE_BUTTON
static bool btnPrev = HIGH;
static unsigned long btnDown = 0;
#endif

// Clock
static unsigned long lastClockTick = 0;

// Flash save
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
  float v = adc * (3.3 / 4095.0);
  if (v >= 3.2 || v < 0.05) return 0;        // open or short circuit
  float r = OIL_R_PULLUP * v / (3.3 - v);    // sender resistance
  float psi = (r - OIL_R_MIN) / (OIL_R_MAX - OIL_R_MIN) * OIL_PSI_MAX;
  return constrain(psi, 0, OIL_PSI_MAX);
}

// ============================================================================
// INIT
// ============================================================================

void sensorsInit() {
  pinMode(OIL_PIN, INPUT);
  pinMode(VOLT_PIN, INPUT);

#if ENABLE_VSS
  pinMode(VSS_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(VSS_PIN), vssISR, RISING);
  lastSpeedCalc = millis();
#endif

#if ENABLE_BUTTON
  pinMode(BUTTON_PIN, INPUT);  // needs external 10K pull-up to 3.3V
#endif

  lastClockTick = millis();
  lastSave = millis();
}

// ============================================================================
// FLASH PERSISTENCE
// ============================================================================

void loadFromFlash() {
  prefs.begin("trip350z", true);
  tripA_km = prefs.getFloat("tA", 0);
  tripB_km = prefs.getFloat("tB", 0);
  clockH   = prefs.getInt("cH", 12);
  clockM   = prefs.getInt("cM", 0);
  prefs.end();
}

void saveToFlash() {
  unsigned long now = millis();
  if (now - lastSave < SAVE_INTERVAL_MS) return;
  lastSave = now;

  prefs.begin("trip350z", false);
  prefs.putFloat("tA", tripA_km);
  prefs.putFloat("tB", tripB_km);
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

void updateSpeed() {
#if ENABLE_VSS
  unsigned long now = millis();
  unsigned long dt = now - lastSpeedCalc;
  if (dt < 500) return;
  lastSpeedCalc = now;

  noInterrupts();
  unsigned long pulses = vssCounter;
  vssCounter = 0;
  interrupts();

  float km = pulses / VSS_PULSES_PER_KM;
  speedKmh = (km / dt) * 3600000.0;
  tripA_km += km;
  tripB_km += km;
#else
  speedKmh = 0;
#endif
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

void handleButton() {
#if ENABLE_BUTTON
  bool cur = digitalRead(BUTTON_PIN);

  if (cur == HIGH && btnPrev == LOW) {
    unsigned long held = millis() - btnDown;
    if (held > 2000) {
      // Long press: reset current trip
      if (displayMode == 0) tripA_km = 0;
      else if (displayMode == 1) tripB_km = 0;
      // Force immediate save
      lastSave = 0;
      saveToFlash();
    } else if (held > 50) {
      // Short press: cycle Trip A → Trip B → Clock
      displayMode = (displayMode + 1) % 3;
    }
  }
  if (cur == LOW && btnPrev == HIGH) btnDown = millis();
  btnPrev = cur;
#endif
}

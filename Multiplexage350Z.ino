// ============================================================================
// 350Z Digital Gauge Replacement
// ESP32 + TCA9548A + 3x SH1106 OLED
//
// Screen 0: Oil Pressure   (sender signal via 100Ω pull-up → GPIO32)
// Screen 1: Battery Voltage (voltage sensor module → GPIO33)
// Screen 2: Trip Computer   (VSS → GPIO34, Button → GPIO35) [optional]
//
// See config.h for wiring diagram, pin assignments, and calibration.
// Set ENABLE_VSS and ENABLE_BUTTON in config.h when you wire those.
// ============================================================================

#include "config.h"
#include "sensors.h"
#include "displays.h"

static unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);

  loadFromFlash();
  sensorsInit();
  displaysInit();
  bootAnimation();

  lastDisplayUpdate = millis();
}

void loop() {
  unsigned long now = millis();

  // Read sensors + handle input every loop
  handleButton();
  updateSpeed();
  updateClock();

  // Refresh displays at ~10 Hz
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL_MS) {
    lastDisplayUpdate = now;

    readOilPressure();
    readBatteryVoltage();

    drawOilGauge();
    drawBatteryGauge();
    drawTripComputer();
  }

  // Persist trip + clock periodically
  saveToFlash();
}

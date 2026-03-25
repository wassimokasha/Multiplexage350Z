// ============================================================================
// 350Z Digital Gauge Replacement
// ESP32 + TCA9548A + 3x SH1106 OLED
//
// Replaces the Triple Meter Assembly (Nissan P/N 24845-CD000) on a 2003 350Z.
// Connects to the car via the stock connector M44.
//
// Screen 0: Oil Pressure   (sensor signal on M44 pin 8 → GPIO32)
// Screen 1: Battery Voltage (M44 pin 3 via voltage sensor → GPIO33)
// Screen 2: Clock + Info    (future: trip data via serial decode of pin 5)
//
// IMPORTANT: You must provide 5V to M44 pin 9 and GND to pin 7 to power
// the oil pressure sensor. See config.h for full wiring diagram.
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

  updateClock();
  processSerialData();

  // Refresh displays at ~10 Hz
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL_MS) {
    lastDisplayUpdate = now;

    readOilPressure();
    readBatteryVoltage();

    drawOilGauge();
    drawBatteryGauge();
    drawInfoScreen();
  }

  // Persist clock periodically
  saveToFlash();
}

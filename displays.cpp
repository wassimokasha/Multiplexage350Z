#include "displays.h"
#include "config.h"
#include "bitmaps.h"
#include "sensors.h"
#include <Wire.h>
#include <U8g2lib.h>

static U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

static int frame = 0;

// ============================================================================
// MUX SELECT
// ============================================================================

static void tcaselect(uint8_t bus) {
  if (bus > 7) return;
  Wire.beginTransmission(MUX_Address);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

// ============================================================================
// INIT + BOOT ANIMATION
// ============================================================================

void displaysInit() {
  Wire.begin();
  for (int i = 0; i < 3; i++) {
    tcaselect(i);
    u8g2.begin();
    u8g2.clearDisplay();
  }
}

void bootAnimation() {
  int carPos = -128;
  int wordPos = -280;

  for (int i = 0; i < 3; i++) {
    tcaselect(i);
    do {
      u8g2.firstPage();
      do {
        u8g2.drawBitmap(carPos, 0, 128 / 8, 64, car_bitmap);
        u8g2.drawBitmap(wordPos, 0, 128 / 8, 64, word_350z);
        wordPos++;
        carPos++;
      } while (u8g2.nextPage());
    } while (carPos < 150 || wordPos < 0);
    carPos = -128;
    wordPos = -280;
    delay(200);
  }
  delay(1000);
}

// ============================================================================
// SCREEN 0 : OIL PRESSURE GAUGE
// ============================================================================

void drawOilGauge() {
  int barW = map(constrain((int)oilPSI, 0, (int)OIL_PSI_MAX),
                 0, (int)OIL_PSI_MAX, 0, 124);

  tcaselect(0);
  u8g2.firstPage();
  do {
    u8g2.drawBitmap(0, 0, 128 / 8, 64, gauge_outline);
    u8g2.drawBox(2, 13, barW, 10);

    // Animated turbo icon spinner
    u8g2.drawBitmap(37, 39, 16 / 8, 9, epd_bitmap_allArray[frame]);

    // Numeric PSI
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", (int)oilPSI);
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(58, 60, buf);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(95, 60, "PSI");
  } while (u8g2.nextPage());
}

// ============================================================================
// SCREEN 1 : BATTERY VOLTAGE
// ============================================================================

void drawBatteryGauge() {
  int segs = constrain((int)battVolts, 0, 16);

  tcaselect(1);
  u8g2.firstPage();
  do {
    u8g2.drawBitmap(0, 0, 128 / 8, 64, battery_voltage);

    for (int i = 0; i < segs; i++) {
      u8g2.drawBox(2 + 6 * i, 13, 5, 10);
    }

    // Numeric voltage
    char buf[8];
    dtostrf(battVolts, 4, 1, buf);
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(40, 60, buf);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(100, 60, "V");
  } while (u8g2.nextPage());
}

// ============================================================================
// SCREEN 2 : INFO DISPLAY (clock + gauges summary)
// ============================================================================
// The original trip computer got its data (speed, trip, DTE, temp, fuel)
// via serial protocol from the unified meter & A/C amp.
// Until that protocol is decoded, this screen shows clock + oil/voltage.
// When ENABLE_TRIP_SERIAL is on and data is decoded, it will show speed
// and outside temperature as well.

void drawInfoScreen() {
  tcaselect(2);
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawHLine(1, 13, 126);

    // Header
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(4, 10, "350Z");

#if ENABLE_TRIP_SERIAL
    if (serialDataValid) {
      u8g2.drawStr(70, 10, "SERIAL OK");
    } else {
      u8g2.drawStr(60, 10, "NO SIGNAL");
    }
#endif

    // Clock (large)
    char clk[8];
    snprintf(clk, sizeof(clk), "%02d:%02d", clockH, clockM);
    u8g2.setFont(u8g2_font_ncenB18_tr);
    u8g2.drawStr(25, 40, clk);

    // Separator
    u8g2.drawHLine(1, 47, 126);

    // Oil pressure + voltage summary
    char line[24];
    snprintf(line, sizeof(line), "%dPSI", (int)oilPSI);
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(4, 62, line);

    char vbuf[8];
    dtostrf(battVolts, 4, 1, vbuf);
    snprintf(line, sizeof(line), "%sV", vbuf);
    u8g2.drawStr(75, 62, line);

  } while (u8g2.nextPage());

  // Advance animation frame
  frame = (frame + 1) % 3;
}

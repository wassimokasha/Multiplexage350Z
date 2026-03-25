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
// SCREEN 2 : TRIP COMPUTER / CLOCK
// ============================================================================

void drawTripComputer() {
  tcaselect(2);
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawHLine(1, 13, 126);

    // Header
    u8g2.setFont(u8g2_font_5x7_tr);
    if (displayMode == 0)      u8g2.drawStr(4, 10, "TRIP A");
    else if (displayMode == 1) u8g2.drawStr(4, 10, "TRIP B");
    else                       u8g2.drawStr(4, 10, "CLOCK");
    u8g2.drawStr(95, 10, "350Z");

    if (displayMode <= 1) {
      // Speed
      char spd[6];
      snprintf(spd, sizeof(spd), "%3d", constrain((int)speedKmh, 0, 299));
      u8g2.setFont(u8g2_font_ncenB18_tr);
      u8g2.drawStr(10, 40, spd);
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(85, 40, "km/h");

      // Trip distance
      u8g2.drawHLine(1, 47, 126);
      float trip = (displayMode == 0) ? tripA_km : tripB_km;
      char dst[12];
      if (trip < 1000) dtostrf(trip, 6, 1, dst);
      else             snprintf(dst, sizeof(dst), "%6d", (int)trip);
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(15, 62, dst);
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(100, 62, "km");

    } else {
      // Clock
      char clk[8];
      snprintf(clk, sizeof(clk), "%02d:%02d", clockH, clockM);
      u8g2.setFont(u8g2_font_ncenB18_tr);
      u8g2.drawStr(20, 40, clk);

      // Summary: voltage + oil
      u8g2.drawHLine(1, 47, 126);
      char info[22];
      snprintf(info, sizeof(info), "%.1fV   %dPSI", battVolts, (int)oilPSI);
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(15, 62, info);
    }
  } while (u8g2.nextPage());

  // Advance animation frame
  frame = (frame + 1) % 3;
}

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

// Current smoothed readings (extern so displays can access them)
extern float oilPSI;
extern float battVolts;
extern float speedKmh;
extern float tripA_km;
extern float tripB_km;

// Clock
extern int clockH, clockM, clockS;

// Display mode (0=Trip A, 1=Trip B, 2=Clock)
extern int displayMode;

void sensorsInit();
void readOilPressure();
void readBatteryVoltage();
void updateSpeed();
void updateClock();
void handleButton();
void loadFromFlash();
void saveToFlash();

#endif

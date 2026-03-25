#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

// Current smoothed sensor readings
extern float oilPSI;
extern float battVolts;

// Clock (persisted to flash)
extern int clockH, clockM, clockS;

// Trip serial data (from unified meter, if decoded)
extern float serialSpeed;       // km/h from unified meter
extern float serialOutsideTemp; // outside air temperature
extern bool  serialDataValid;   // true if we're receiving valid data

void sensorsInit();
void readOilPressure();
void readBatteryVoltage();
void updateClock();
void processSerialData();
void loadFromFlash();
void saveToFlash();

#endif

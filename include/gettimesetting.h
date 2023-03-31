#ifndef gettimesetting_h

#define gettimesetting_h
#include <LittleFS.h>

// the #include statement and code go here...

// Initialize rtc variable and tell sketch to use it on every links
void getTimesetting(uint8_t &waktu1Awal, uint8_t &waktu2Awal, uint8_t &waktu1Akhir, uint8_t &waktu2Akhir);

#endif
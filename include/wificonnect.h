#ifndef wificonnect_h

#define wificonnect_h
#include <ESP8266WiFi.h>

// the #include statement and code go here...

void wificonnect(const char* ssid, const char* password, const char* deviceName, uint8_t connled);

#endif
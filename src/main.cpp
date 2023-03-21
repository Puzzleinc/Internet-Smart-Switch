#include <Arduino.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "wificonnect.h"

/* Put your SSID & Password */
const char* ssid;  // Enter SSID here
const char* password;  //Enter Password here
const char* deviceName;

// Initialize Internal led build in
uint8_t connled = 2;
uint8_t saklar = 13;

// Iniotialize NTP Class
WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// NTPClient timeClient(ntpUDP);

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
// NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // Menambahkan variabel langsung pada fungsi
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Setel waktu alarm ( 2 waktu sekaligus )
uint8_t jamTimer[][2] = {{9, 11},{17, 20}};   

// Clock variable
unsigned long timeLast = 0;
unsigned long previousMillis = 0;
unsigned long intervalDays = 2*24*60*60*1000; // 2 Days interval
uint8_t intervalSec = 2; // 2 Days interval

// set your starting hour here, not below at int hour. This ensures accurate daily correction of time
uint8_t seconds;
uint8_t minutes;
uint8_t hours;
uint16_t days;

void updateTime(unsigned long currentMillis) {
  // update waktu ke server NTP
  if (currentMillis - timeLast >= intervalDays) {
    timeLast = currentMillis;
    
    timeClient.update();
    seconds = timeClient.getSeconds();
    minutes = timeClient.getMinutes();
    hours = timeClient.getHours();
  }
}

void showTime(unsigned long currentMillis) {
  // tampilkan waktu saat ini ke serial monitor
  if (currentMillis - timeLast >= intervalSec) {
    timeLast = currentMillis;
    
    timeClient.update();
    Serial.print("Waktu internal: ");
    Serial.print(days);
    Serial.print(":");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    Serial.print(" -- ");
    Serial.println("Waktu internet : " + timeClient.getFormattedTime());
  }
}

void clockCounter(unsigned long currentMillis) {
  seconds = currentMillis - previousMillis;

  //the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds == 60) {
    previousMillis = currentMillis;
    seconds = 0;
    minutes = minutes + 1; 
  }

  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes == 60){
    minutes = 0;
    hours = hours + 1; 
  }

  if (hours == 24){
    hours = 0;
    days = days + 1; 
  }
}

void timerFunction() {
  // Fungsi timer utama
  for (int i=0; i<2; i++) {
    if(hours >= jamTimer[i][0] && hours <= jamTimer[i][1]-1) {
      // Serial.println("Alarm aktif");
        digitalWrite(saklar, HIGH);
      } else {
        digitalWrite(saklar, LOW);
    }
  }
}

void setup() {
 /*  Common cathode led RGB */
  Serial.begin(115200);

  //  Initialize IO pin -----------------------
  pinMode(connled, OUTPUT);
  pinMode(saklar, OUTPUT);

  //  Connecting to wifi -----------------------
  ssid = "Puzzle24";  // Enter SSID here
  password = "gzcmb94463";  // Enter Password here
  deviceName = "Sensor Pompa"; // DHCP Hostname (useful for finding device for static lease)
  wificonnect(ssid, password, deviceName, connled);

  Serial.println(WiFi.gatewayIP());

  timeClient.begin();
  // Tambahan fungsi
  timeClient.setTimeOffset(25200);
  timeClient.setUpdateInterval(180000);

  // Dapatkan waktu pertama kali dari NTP Server
  timeClient.update();
  seconds = timeClient.getSeconds();
  minutes = timeClient.getMinutes();
  hours = timeClient.getHours();
}

void loop() {
  // Millis Function #1
  unsigned long currentMillis = millis() / 1000;  // the number of milliseconds that have passed since boot
  
  clockCounter(currentMillis);
  updateTime(currentMillis);
  showTime(currentMillis);
  timerFunction();
}
#include "gettimesetting.h"

void getTimesetting(uint8_t &waktu1Awal, uint8_t &waktu2Awal, uint8_t &waktu1Akhir, uint8_t &waktu2Akhir) {
    // Get data from SPIFFS and send it to global variable

    String inputWaktu = "";
    File file = LittleFS.open("time.csv", "r");

    if(!file){
    Serial.println("Failed to open file for reading");
    return;
    }

    while (file.available()) {
        inputWaktu += (char)file.read();
    }

    uint8_t pembatas1 = inputWaktu.indexOf("#");
    uint8_t pembatas2 = inputWaktu.indexOf("-");
    uint8_t pembatas3 = inputWaktu.lastIndexOf("#");

    waktu1Awal = inputWaktu.substring(0, pembatas1).toInt();
    waktu1Akhir = inputWaktu.substring(pembatas1+1, inputWaktu.length()).toInt();
    
    waktu2Awal = inputWaktu.substring(pembatas2+1, inputWaktu.length()).toInt();
    waktu2Akhir = inputWaktu.substring(pembatas3+1, inputWaktu.length()).toInt();
    file.close();
}
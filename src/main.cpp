#include <Arduino.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <CTBot.h>

#include "gettimesetting.h"
#include "wificonnect.h"

/* WIFI SSID & Password */
const char* ssid;  // Enter SSID here
const char* password;  //Enter Password here
const char* deviceName;

/*  GPIO SETUP */
// Init pin number on ESP8266 board
#define connled 2
#define saklar 13
#define secIndicator 12

// Init pin number on ESP01 board
// #define buzz 1 //TX
// #define saklar 3 // RX / Saklar
// #define connled 0
// #define secIndicator 2

/*  NTP SETUP */
// Iniotialize NTP Class
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

  uint8_t waktu1Awal;
  uint8_t waktu1Akhir;
  
  uint8_t waktu2Awal;
  uint8_t waktu2Akhir;

// Clock variable
unsigned long timeLast = 0;
unsigned long previousMillis = 0;
const unsigned long intervalDays = 2*24*60*60*1000; // 2 Days interval
uint8_t const intervalSec = 5; // 2 Days interval

// set your starting hour here, not below at int hour. This ensures accurate daily correction of time
uint8_t seconds;
uint8_t minutes;
uint8_t hours;
uint16_t days;

/*  TELEGRAM BOT SETUP */
CTBot myBot;
TBMessage msg;
String token = "5818046411:AAEv_uG0gr_Sn3eUncC5cMo3IkvBVsR73Pk";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
CTBotInlineKeyboard myKbd;   // reply keyboard object helper

/* Init CTBot query */
#define LIGHT_ON_CALLBACK  "ON1"  // callback data sent when "LIGHT ON" button is pressed
#define LIGHT_OFF_CALLBACK "OFF1" // callback data sent when "LIGHT OFF" button is pressed 

// Make function prototype
void updateTime(unsigned long currentMillis);
void showTime(unsigned long currentMillis);
void clockCounter(unsigned long currentMillis);
void timerFunction();
void telegramOperation(unsigned long currentTime);
void telegramKeyboard();


void setup() {
 /*  Common cathode led RGB */
  Serial.begin(115200);

  //  Initialize IO pin -----------------------
  pinMode(connled, OUTPUT);
  pinMode(saklar, OUTPUT);
  pinMode(secIndicator, OUTPUT);

  //  Connecting to wifi -----------------------
  ssid = "Puzzle24";  // Enter SSID here
  password = "gzcmb94463";  // Enter Password here
  deviceName = "Smart Switch"; // DHCP Hostname (useful for finding device for static lease)
  wificonnect(ssid, password, deviceName, connled);

  Serial.println(WiFi.gatewayIP());

  	// Initialize filesystem -----------------------
	if(!LittleFS.begin()){
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}

	// Testing filesystem -----------------------
	File file = LittleFS.open("time.csv", "r");
	if(!file){
		Serial.println("Failed to open file for reading");
		return;
	}
	Serial.println("File Content:");
	while(file.available()){
		Serial.write(file.read());
	}
	file.close();

  // Get time setting from filesystem -----------------------
	getTimesetting(waktu1Awal, waktu2Awal, waktu1Akhir, waktu2Akhir);

	//  Connecting to telegram -----------------------
	myBot.setTelegramToken(token);
	// check if all things are ok
	if (myBot.testConnection())
		Serial.println("\nTest connection Succes");
	else
		Serial.println("\nTest connection Failed");

  //  NTP Server begin -----------------------
  timeClient.begin();
  // Tambahan fungsi
  timeClient.setTimeOffset(25200);
  timeClient.setUpdateInterval(180000);

  // Dapatkan waktu pertama kali dari NTP Server -----------------------
  timeClient.update();
  seconds = timeClient.getSeconds();
  minutes = timeClient.getMinutes();
  hours = timeClient.getHours();

  // Calling Keyboard button class -----------------------
	telegramKeyboard();

  // String timerstartStr = String(jamTimer[0][0]);
  // String timerstopStr = String(jamTimer[0][1]);
  // Serial.println(timerstartStr);
  // Serial.println(timerstopStr);
}

void loop() {
  // Millis Function #1
  unsigned long currentMillis = millis() / 1000;  // the number of milliseconds that have passed since boot
  
  telegramOperation(currentMillis);
  clockCounter(currentMillis);
  updateTime(currentMillis);
  showTime(currentMillis);
  timerFunction();
}

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
  if (currentMillis - timeLast >= intervalSec) {
    timeLast = currentMillis;

    // Indikator waktu tiap detik led menyala
    digitalWrite(secIndicator, HIGH);
    delay(50);
    digitalWrite(secIndicator, LOW);
    delay(50);

    // tampilkan waktu saat ini ke serial monitor
    // timeClient.update();
    // Serial.print("Waktu internal: ");
    // Serial.print(days);
    // Serial.print(":");
    // Serial.print(hours);
    // Serial.print(":");
    // Serial.print(minutes);
    // Serial.print(":");
    // Serial.println(seconds);
    // Serial.print(" -- ");
    // Serial.println("Waktu internet : " + timeClient.getFormattedTime());
  }
}

void clockCounter(unsigned long currentMillis) {
  seconds = currentMillis - previousMillis;

  //the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds >= 60) {
    previousMillis = currentMillis;
    seconds = 0;
    minutes = minutes + 1; 
  }

  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes >= 60){
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
    if((hours >= waktu1Awal && hours <= waktu1Akhir-1) || (hours >= waktu2Awal && hours <= waktu2Akhir-1)) {
      // MEnggunakan Relay NORMALLY CLOSED, jadi waktu nyala akan putus dan waktu mati akan nyambung
        digitalWrite(saklar, LOW);
      } else {
        digitalWrite(saklar, HIGH);
    }
  }
}

void telegramOperation(unsigned long currentMillis) {
	TBMessage msg;

	// 3 Relay switch
	String welcome = "Halo\n";
	welcome += "Ini adalah bot untuk app esp8266\n";
	welcome += "ketik /Keyboard untuk menampilkan menu\n";
	welcome += "ketik /Time untuk waktu timer berjalan\n";
	welcome += "ketik /CheckTime untuk melihat catatan waktu tersimpan\n";
	welcome += "Format pengaturan waktu alarm: WAKTUAWAL1#WAKTUAKHIR1-WAKTUAWAL2#WAKTUAKHIR2\n\n";

  String hoursStr = String(hours);
  String minutesStr = String(minutes);
  String secondsStr = String(seconds);

	if(myBot.getNewMessage(msg)) {
		if(msg.messageType == CTBotMessageText) {
			if(msg.text.equalsIgnoreCase("/Start") || msg.text.equalsIgnoreCase("/Keyboard")) {
				myBot.sendMessage(msg.sender.id, welcome, myKbd);
			} else if (msg.text.equalsIgnoreCase("/Time")) {
				myBot.sendMessage(msg.sender.id, "Waktu internal: " + hoursStr + ":" + minutesStr + ":" + secondsStr);
			} else if (msg.text.equalsIgnoreCase("/CheckTime")) {
        getTimesetting(waktu1Awal, waktu2Awal, waktu1Akhir, waktu2Akhir);
				myBot.sendMessage(msg.sender.id, "Waktu Timer #1: " + String(waktu1Awal) + " sampai " + String(waktu1Akhir) + " - Waktu Timer #2: " + String(waktu2Awal) + " sampai " + String(waktu2Akhir));
        } else {
				String inputWaktu = msg.text;

        uint8_t pembatas1 = inputWaktu.indexOf("#");
        uint8_t pembatas2 = inputWaktu.indexOf("-");
        uint8_t pembatas3 = inputWaktu.lastIndexOf("#");

        uint8_t waktu1Awal = inputWaktu.substring(0, pembatas1).toInt();
        uint8_t waktu1Akhir = inputWaktu.substring(pembatas1+1, inputWaktu.length()).toInt();
        
        uint8_t waktu2Awal = inputWaktu.substring(pembatas2+1, inputWaktu.length()).toInt();
        uint8_t waktu2Akhir = inputWaktu.substring(pembatas3+1, inputWaktu.length()).toInt();

        if ((waktu1Awal < waktu1Akhir)||(waktu2Awal < waktu2Akhir)) {
          File file = LittleFS.open("time.csv", "w");
					//Write to the file
					file.print(inputWaktu);
					delay(1);
					//Close the file
					file.close();

					getTimesetting(waktu1Awal, waktu2Awal, waktu1Akhir, waktu2Akhir);
					myBot.sendMessage(msg.sender.id, "Waktu Timer #1: " + String(waktu1Awal) + " sampai " + String(waktu1Akhir) + " - Waktu Timer #2: " + String(waktu2Awal) + " sampai " + String(waktu2Akhir));
					myBot.sendMessage(msg.sender.id, "Pengaturan tersimpan, harap mereset perangkat.....");
        } else {
          myBot.sendMessage(msg.sender.id, "Error, format waktu tidak valid atau menit ke 2 lebih kecil daripada menit ke 1");
        }
			}
		} else if (msg.messageType == CTBotMessageQuery) {
			if (msg.callbackQueryData.equals(LIGHT_ON_CALLBACK)) {
				// pushed "LIGHT ON" button...
				digitalWrite(saklar, LOW);
				// terminate the callback with an alert message
				myBot.endQuery(msg.callbackQueryID, "Light on", true);
			} else if (msg.callbackQueryData.equals(LIGHT_OFF_CALLBACK)) {
				// pushed "LIGHT OFF" button...
				digitalWrite(saklar, HIGH);
				// terminate the callback with a popup message
				myBot.endQuery(msg.callbackQueryID, "Light off", true);
			}
		}
	}
}

void telegramKeyboard() {
	//  Creating Keyboard button class -----------------------
	myKbd.addButton("Lampu Nyala", LIGHT_ON_CALLBACK, CTBotKeyboardButtonQuery);
	myKbd.addButton("Lampu Mati", LIGHT_OFF_CALLBACK, CTBotKeyboardButtonQuery);
	myKbd.addRow();
	myKbd.addButton("Contact request", "https://github.com/shurillu/CTBot", CTBotKeyboardButtonURL);
}
#include "wificonnect.h"

void wificonnect(const char* ssid, const char* password, const char* deviceName, uint8_t connled) {
    WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)

    // Set your Static IP address ------------------------
    IPAddress local_IP(192, 168, 0, 248);
    // Set your Gateway IP address
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(1, 1, 1, 1);   //optional
    IPAddress secondaryDNS(1, 0, 0, 1); //optional
    
    // Configures static IP address ----------------------
    // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //     Serial.println("STA Failed to configure");
    // }

    // Mengatur WiFi -----------------------
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password); // Mencocokan SSID dan Password
    WiFi.mode(WIFI_STA);  //WiFi mode station (connect to wifi router only
        
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(connled, HIGH);
        delay(100);
        digitalWrite(connled, LOW);
        delay(100);
    }

    Serial.println("WiFi connected");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
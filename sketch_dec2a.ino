/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-mysql
 */

// #include <SPI.h>
// #include <Ethernet.h>
#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include "config.h"
MKRIoTCarrier carrier;


char ssid[] = WIFI_NAME;      // your network SSID (name)
char pass[] = WIFI_PASSWORD;  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;

// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.0.52"; // change to your PC's IP address
String PATH_NAME   = "/insert_temp.php";
String queryString = "?temperature=29.1";

void setup() {
  Serial.begin(9600);
  carrier.withCase();
  setupWiFi();

    carrier.begin();

  // initialize the Ethernet shield using DHCP:
  // if (Ethernet.begin(mac) == 0) {
  //   Serial.println("Failed to obtaining an IP address using DHCP");
  //   while(true);
  // }

  // connect to web server on port 80:
  if(wifiClient.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    wifiClient.println(HTTP_METHOD + " " + PATH_NAME + queryString + " HTTP/1.1");
    wifiClient.println("Host: " + String(HOST_NAME));
    wifiClient.println("Connection: close");
    wifiClient.println(); // end HTTP header

    while(wifiClient.connected()) {
      if(wifiClient.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = wifiClient.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the wifiClient:
    wifiClient.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
}

void loop() {

}




void setupWiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(INTERVAL);
  }
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
}

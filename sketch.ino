// Needs adjustment time for sound sensor


#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ThingSpeak.h>
#include "config.h"
#include <Arduino_APDS9960.h>

MKRIoTCarrier carrier;

unsigned long myChannelNumber = SECRET_CH_ID;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;

char ssid[] = WIFI_NAME;      // your network SSID (name)
char pass[] = WIFI_PASSWORD;  // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
const char* mqttServer = "mqtt3.thingspeak.com";  // Replace with your MQTT Broker address
const int mqttPort = 1883;                        // typical MQTT port

WiFiClient wifiClient;

float Gx, Gy, Gz;
int shake_event = 0;
unsigned long lastMoved = 0;
unsigned long timeSinceMovement = 0;
unsigned long currentMillis;
unsigned long previousMillis = 0;
const unsigned long period = 1000;
unsigned long currentTime;
unsigned long currentDateTime;
int hr;
int mn;
int ledColour;
int timeSinceMove;
int startTime = 9;
int endTime = 17;
const int sound_sensor = A0;
int soundValue = 0;
int soundNotification = 0;
int soundMonitoring = 0;
int lastSoundMonitoring = 0;

const long mySQLinterval = 10000;  // interval at which to upload average sound data to mySQL (milliseconds)


//find out what this is for. PHP - my sql. have not updated it.
// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//php-mysql
int HTTP_PORT = 80;
String HTTP_METHOD = "GET";
char HOST_NAME[] = "192.168.0.52";  // change to your PC's IP address
String PATH_NAME = "/workClerk/insert_readings2.php";
String queryString = "?sound=28.2";


int soundValueTotal = 0;
int numValues = 0;
int soundAvg;
float soundChange;


void setup() {
  Serial.begin(9600);
  setupWiFi();
  ThingSpeak.begin(wifiClient);
  carrier.begin();
  getCurrentTime();
  LEDoff();
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor!");
  }
  delay(1000);
  Serial.println("End of Setup");
}

void loop() {


  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  soundValue = analogRead(sound_sensor);

  if (APDS.gestureAvailable()) {
    // a gesture was detected, read and print to Serial Monitor
    int gesture = APDS.readGesture();

    if (gesture == GESTURE_UP || gesture == GESTURE_DOWN || gesture == GESTURE_LEFT || gesture == GESTURE_RIGHT) {
      if ( currentMillis < 60000 ) {
        Serial.println("Delay of 60sec before sound monitor works");
      } else {
      controlSoundMonitoring();
    }
      delay(3000);
    }
  }

 
  if (lastSoundMonitoring == 0 && soundMonitoring == 1) {
    Serial.println("Start Monitoring Sound");
  }

  if (lastSoundMonitoring == 1 && soundMonitoring == 0) {
    Serial.println("End Monitoring Sound");
  }
 lastSoundMonitoring = soundMonitoring;
  if (soundMonitoring == 1) {
  
    soundValueTotal = soundValueTotal + soundValue;
    numValues = numValues + 1;
    soundAvg = soundValueTotal / numValues;
    soundChange = round(((soundValue - soundAvg) * 100) / soundAvg) / 100;

    //if the sound change is 10% more than the average, trigger
    if (soundChange > 0.20) {

      //print some values
      Serial.print("sound: ");
      Serial.print(soundValue);
      Serial.print("         avg: ");
      Serial.print(soundAvg);
      Serial.print("         percent: ");
      Serial.print(soundChange);
      Serial.println();
      
      soundThingSpeak(); //write to Thinkspeak which triggers Alexa
      delay(15000);  //delay listening to any more sound or events for 15 seconds

    }

    //write data to mySQL via php
    if (currentMillis - previousMillis >= mySQLinterval) {
      previousMillis = currentMillis;  // save the last time you saved data
      writeToMySQL();
    }
  }
}



void writeToMySQL() {

  // connect to web server on port 80:
  if (wifiClient.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    wifiClient.println(HTTP_METHOD + " " + PATH_NAME + "?sound=" + soundAvg + " HTTP/1.1");
    wifiClient.println("Host: " + String(HOST_NAME));
    wifiClient.println("Connection: close");
    wifiClient.println();  // end HTTP header

    while (wifiClient.connected()) {
      if (wifiClient.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = wifiClient.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the wifiClient:
    wifiClient.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {  // if not connected:
    Serial.println("connection failed");
  }
}


void soundThingSpeak() {
  soundNotification = 1;
  ThingSpeak.setField(3, soundNotification);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  soundNotification = 0;  //reset variable
}



void writeThinkSpeak() {

  // read the sensor values
  float temperature = carrier.Env.readTemperature();

  // set the fields with the values
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, shake_event);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
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


void getCurrentTime() {
  currentDateTime = WiFi.getTime();
  currentTime = currentDateTime % 86400;  //remainder operation - means current time only contains time portion of current datetime (divisor is number of seconds in 1 day)
  hr = currentTime / 3600;                //divide current time by # seconds in an hour to get the hour of the day, as this is an integer, it will ignore decimel places, and this will be handled by the mn variable
  mn = (currentTime % 3600) / 60;         //remainder of seconds after the hour is taken away

  Serial.println("currentTime");
  Serial.println(currentTime);
  Serial.println("hr");
  Serial.println(hr);
  Serial.println("mn");
  Serial.println(mn);
}


void LEDon() {
  //temperarily comment below while testing other things
  ledColour = carrier.leds.Color(200, 197, 45);
  carrier.leds.fill(ledColour);
  carrier.leds.show();
}


void LEDoff() {
  carrier.leds.fill(0);  //Set all LEDs to no colour(off)
  carrier.leds.show();   //update the new state of the LEDs
}


void resetSound() {
}

void controlSoundMonitoring() {
  if (soundMonitoring == 0) {
    soundMonitoring = 1;
    LEDon();
  } else if (soundMonitoring == 1) {
    soundMonitoring = 0;
    LEDoff();
  }
}

// Needs adjustment time for sound sensor

//Blynk Device Info
#define BLYNK_TEMPLATE_ID "TMPL4J8QUatXs"
#define BLYNK_TEMPLATE_NAME "HDipIOTAssignmentTemplate"
#define BLYNK_AUTH_TOKEN "z8Xpu35yseI6mcEpPx0QLJ6UZr--6_x8"


#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ThingSpeak.h>
#include "config.h"
#include <Arduino_APDS9960.h>
#include <BlynkSimpleWifi.h>
#include <ArduinoHttpClient.h>


MKRIoTCarrier carrier;

BlynkTimer timer;

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
unsigned long previousMillisTemp = 0;

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

const long mySQLinterval = 60000;  // interval at which to upload average sound data to mySQL (milliseconds)
const long tempInterval = 5000;    // interval at which to read and analyse temperature data (milliseconds)



//find out what this is for. PHP - my sql. have not updated it.
// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//php-mysql
int HTTP_PORT = 80;
String HTTP_METHOD = "GET";
char HOST_NAME[] = "192.168.0.52";  // change to your PC's IP address

String insertSoundValuePath = "/workClerk/insert_sound_readings.php";
String insertTemperaturePath = "/workClerk/insert_temperature_readings.php";
String insertSoundMonitoringPath = "/workClerk/insert_sound_monitoring_readings.php";

String readPath = "/workClerk/get_readings4.php";
// String queryString = "?sound=28.2";

HttpClient httpclient = HttpClient(wifiClient, HOST_NAME, HTTP_PORT);


int soundValueTotal = 0;
int numValues = 0;
int soundAvg;
float soundChange;
const float tempThreshold = 20.0;
float temperature;
int tempWarning = 0;
int lastTempWarning = 0;


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

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function to be called every second
  timer.setInterval(2000L, writeBlynkTemp);

  delay(1000);
  Serial.println("End of Setup");
}



//***************************************************************************************************************************************************************************
//***************************************************************************************************************************************************************************

//                LOOP

//***************************************************************************************************************************************************************************
//***************************************************************************************************************************************************************************

void loop() {


  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  soundValue = analogRead(sound_sensor);


  //write data to mySQL via php
  if (currentMillis - previousMillisTemp >= tempInterval) {
    previousMillisTemp = currentMillis;  // save the last time you saved data
    temperature = carrier.Env.readTemperature();
    writeToMySQL("insertTemperaturePath");
  }


  Blynk.run();
  timer.run();

  if (APDS.gestureAvailable()) {
    // a gesture was detected, read and print to Serial Monitor
    int gesture = APDS.readGesture();

    if (gesture == GESTURE_UP || gesture == GESTURE_DOWN || gesture == GESTURE_LEFT || gesture == GESTURE_RIGHT) {
      // if ( currentMillis < 60000 ) {
      //   Serial.println("Delay of 60sec before sound monitor works");
      // } else {
      controlSoundMonitoring();
      writeToMySQL("insertSoundMonitoringPath");
      // }
    }
  }

  if (lastSoundMonitoring == 0 && soundMonitoring == 1) {
    Serial.println("Start Monitoring Sound");
    LEDon();
    writeBlynkSoundMonitor();
    delay(3000);
  }

  if (lastSoundMonitoring == 1 && soundMonitoring == 0) {
    Serial.println("End Monitoring Sound");
    LEDoff();
    writeBlynkSoundMonitor();
    delay(3000);
  }
  lastSoundMonitoring = soundMonitoring;
  if (soundMonitoring == 1) {

    soundValueTotal = soundValueTotal + soundValue;
    numValues = numValues + 1;
    soundAvg = soundValueTotal / numValues;
    soundChange = round(((soundValue - soundAvg) * 100) / soundAvg) / 100;

    //print some values
    // Serial.print("sound: ");
    // Serial.print(soundValue);
    // Serial.print("         avg: ");
    // Serial.print(soundAvg);
    // Serial.print("         percent: ");
    // Serial.print(soundChange);
    // Serial.println();


    //if the sound change is 10% more than the average, trigger
    if (soundChange > 0.20) {

      soundThingSpeak();  //write to Thinkspeak which triggers Alexa
      delay(15000);       //delay listening to any more sound or events for 15 seconds
    }

    //write data to mySQL via php
    if (currentMillis - previousMillis >= mySQLinterval) {
      previousMillis = currentMillis;  // save the last time you saved data
      writeToMySQL("insertSoundValuePath");
      // writeToMySQL();
    }
  }

  // Decide how often or why I want to do this
  // readMySQLData();


  //Check if there's been a change in temperature warning
  checkTemperature();

  if (lastTempWarning == 0 && tempWarning == 1) {
    Serial.println("Temperature above threshold");
    carrier.display.fillScreen(0xF800);
    delay(3000);
  }

  if (lastTempWarning == 1 && tempWarning == 0) {
    Serial.println("Temperature below threshold");
    carrier.display.fillScreen(0x0000);
    delay(3000);
  }
  lastTempWarning = tempWarning;
}


//***************************************************************************************************************************************************************************
//***************************************************************************************************************************************************************************

//                FUNCTIONS

//***************************************************************************************************************************************************************************
//***************************************************************************************************************************************************************************

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0) {
  soundMonitoring = param.asInt();
}


// This function sends temperature every second to Virtual Pin 1.
void writeBlynkSoundMonitor() {
  // Don't send more that 10 values per second.
  Blynk.virtualWrite(V0, soundMonitoring);
}


void writeBlynkTemp() {
  // Don't send more that 10 values per second.
  Blynk.virtualWrite(V1, temperature);
}


void writeToMySQL(String writeType) {

  // connect to web server on port 80:
  if (wifiClient.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    if (writeType == "insertSoundValuePath") {
      wifiClient.println(HTTP_METHOD + " " + insertSoundValuePath + "?sound=" + soundAvg + " HTTP/1.1");
    } else if (writeType == "insertTemperaturePath") {
      wifiClient.println(HTTP_METHOD + " " + insertTemperaturePath + "?temperature=" + temperature + " HTTP/1.1");
    } else if (writeType == "insertSoundMonitoringPath") {
      wifiClient.println(HTTP_METHOD + " " + insertSoundMonitoringPath + "?monitoring_value=" + soundMonitoring + " HTTP/1.1");
    }
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


void readMySQLData() {

  httpclient.get(readPath);

  // read the response
  String response = httpclient.responseBody();
  Serial.println("MySQL Response:");
  Serial.println(response);
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
  ledColour = carrier.leds.Color(200, 197, 45);
  carrier.leds.fill(ledColour);
  carrier.leds.show();
}


void LEDoff() {
  carrier.leds.fill(0);  //Set all LEDs to no colour(off)
  carrier.leds.show();   //update the new state of the LEDs
}


void controlSoundMonitoring() {
  if (soundMonitoring == 0) {
    soundMonitoring = 1;
  } else if (soundMonitoring == 1) {
    soundMonitoring = 0;
  }
}

void checkTemperature() {

  if (temperature > tempThreshold) {
    tempWarning = 1;
  } else {
    tempWarning = 0;
  }
}

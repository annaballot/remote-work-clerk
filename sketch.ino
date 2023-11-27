#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ThingSpeak.h>
#include "config.h"
MKRIoTCarrier carrier;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

char ssid[] = WIFI_NAME;        // your network SSID (name)
char pass[] = WIFI_PASSWORD;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
const char* mqttServer = "mqtt3.thingspeak.com";  // Replace with your MQTT Broker address
const int mqttPort = 1883;                  // typical MQTT port

WiFiClient wifiClient;

float Gx, Gy, Gz;
int shake_event = 0;
unsigned long lastMoved = 0;
unsigned long timeSinceMovement = 0;
unsigned long currentMillis;
const unsigned long period = 1000;
unsigned long currentTime;
unsigned long currentDateTime;
int hr;
int mn;
int ledColour;
int timeSinceMove;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  carrier.withCase();
  setupWiFi();
  ThingSpeak.begin(wifiClient);
  carrier.begin();


getCurrentTime();

LEDoff();

  carrier.display.setRotation(0);
  delay(1000);
  Serial.println("Start of Monitoring");
}

void loop() {

currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
timeSinceMove = currentMillis/1000 - lastMoved;


// read the IMU values
  carrier.IMUmodule.readGyroscope(Gx, Gy, Gz);

    if (Gy > 100 || Gy < -100) {  //update this, not v accurate
      shake_event = 1;
      LEDoff();
      Serial.println("Intruder");
      writeThinkSpeak();
      lastMoved = currentMillis/1000;
      
      //delays for 15 seconds before  continuing with the loop
       delay(15000);
       //reset shake event
        shake_event = 0;
    }
 


Serial.println("Time Since Moved");
 Serial.println(timeSinceMove);

if (timeSinceMove > 60) {

LEDon();

}

// timeSinceMovement = millis() - lastMoved;
//  Serial.println(timeSinceMovement);


}


void writeThinkSpeak() {


  // read the sensor values
  float temperature = carrier.Env.readTemperature();

    // set the fields with the values
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, shake_event);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }





}




void setupWiFi() {
 // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
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
  hr = currentTime/3600; //divide current time by # seconds in an hour to get the hour of the day, as this is an integer, it will ignore decimel places, and this will be handled by the mn variable
  mn = (currentTime % 3600)/60; //remainder of seconds after the hour is taken away

// Serial.println("currentDateTime");
// Serial.println(currentDateTime);
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
  carrier.leds.fill(0); 			//Set all LEDs to no colour(off)
  carrier.leds.show(); 				//update the new state of the LEDs 
}

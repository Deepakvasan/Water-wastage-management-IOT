#include <ESP8266WiFi.h>
#include "ThingSpeak.h"


String writeAPIKey = "";
const char *ssid = "";   
const char *pass = "";
const char *server = "api.thingspeak.com";
unsigned long myChannelNumber = xxxxxxxxxx;  // enter the channel number here
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 7.5;
volatile byte pulseCount,pulseCount_1;
byte pulse1Sec=0;
byte pulse1Sec_1 = 0;
float flowRate,flowRate_1;
float flowLitres,flowLitres_1;
float totalLitres,totalLitres_1;
long currentMillis_1 = 0;
long previousMillis_1 = 0;



int triggerPin = 8;
int echoPin = 9;
int tankCapacity = 500;   // tank's total capacity in litres....
float tankHeight = 1.5;   //height of the tank in metres...
float h = 0.2;   //height in which the Ultrasonic sensor is place above the water holding capacity of the tank....
float r  = 0.5;  // radius of the tank ....
float waterUsage = 0.0;  // to store the usage of water

WiFiClient client;


void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}


void IRAM_ATTR pulseCounter_1()
{
  pulseCount_1++;
}



void setup() {
  // put your setup code here, to run once:
    pulseCount = 0;
    flowRate = 0.0;
    previousMillis = 0;
    pulseCount_1 = 0;
    flowRate_1 = 0.0;
    previousMillis_1 = 0.0;
//    pinMode(LED_BUILTIN, OUTPUT);
//    pinMode(SENSOR, INPUT_PULLUP);
    WiFi.mode(WIFI_STA); 
    ThingSpeak.begin(client); 
    pinMode(triggerPin,OUTPUT);
    pinMode(echoPin,INPUT);
    Serial.begin(115200);  // here 115200 is the baud rate for the esp module...
    attachInterrupt(/*digitalPinToInterrupt */, pulseCounter, FALLING);   
    attachInterrupt(/*digitalPinToInterrupt */, pulseCounter_1, FALLING);    
}





float waterFlownTap(){
      // code for water flow meter starts from here....
   currentMillis_1 = millis();
  if (currentMillis_1 - previousMillis_1 > interval) 
  {
    pulse1Sec_1 = pulseCount_1;
    pulseCount_1 = 0;
    flowRate = ((1000.0 / (millis() - previousMillis_1)) * pulse1Sec_1) / calibrationFactor;
    previousMillis_1 = millis();
    flowLitres = (flowRate_1 / 60);
    totalLitres_1 += flowLitres_1;
    return totalLitres_1;
  }
  return 0.0;
}






float overFlow(){
        // code for water flow meter starts from here....
    currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowLitres = (flowRate / 60);
    totalLitres += flowLitres;
    return totalLitres;
  }
  return 0.0;
  
}






float waterUsed(){
  float distance = 0.0;
  float duration = 0.0;
  float waterLevel;  // water level in meter's inside the tank...
  float volumeOfWater;  // amount of water in litres...
  digitalWrite(triggerPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin,LOW);
  duration = pulseIn(echoPin,HIGH);
  distance = (duration/2)*0.000343;  // distance in meters
  waterLevel = (tankHeight+h)-distance;
  volumeOfWater = (3.14)*(r*r)*waterLevel*1000 ; // which gives the amount of water present inside the tank in litres
  waterUsage = tankCapacity - volumeOfWater;
  return waterUsage;
}





void loop() {
  // put your main code here, to run repeatedly:


  // code for ultrasonic sensor begins from here

    // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(*ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

 int waterUsage = waterUsed();
 int tapWater = waterFlownTap();
 int overflowed = overFlow();
 
 int x = ThingSpeak.writeField(myChannelNumber,1,waterUsage,writeAPIKey);   // here 1 is the value that denotes the field 1..... // this one upload the waterflow meter's result
 int y = ThingSpeak.writeField(myChannelNumber,2,tapWater,writeAPIKey);       // this one upload the ultrasonic sensor's result to the thingspeak server...
 int z = ThingSpeak.writeField(myChannelNumber,3,overflowed,writeAPIKey);   // this one upload the amount of water wasted due to overflow....

 if (int x == 200){  // here the above function return 200 if the value is updated to the server successfully...
    Serial.println("Water flow meter's output is updated successfully");
 }
 else{
    Serial.println("Updating the result of flow meter is failed");
 }

 if (y == 200){
    Serial.println("Ultrasonic sensor's output is updated successfully");
 }

 else{
    Serial.println("Updating the result of ultrasonic sensor is failed");
 }

 
 if (z == 200){
    Serial.println("amount of water overflowed, updated successfully");
 }

 else{
    Serial.println("Updating the result of overflowed water is unsuccessful");
 }

}

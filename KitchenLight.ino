/*************************************************************
  Control the LED lamp with Blynk and ultrasonic distance
 *************************************************************/
#define BLYNK_PRINT Serial // Defines the object that is used for printing

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "cie9131.h"

BlynkTimer timer; // Announcing the timer

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "Token: 1b19d*****************";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WiFi SSID";
char pass[] = "WiFi Password";

long duration, distance;
bool ledState = HIGH;
int distanceSet = 0;
bool Connected2Blynk = false;
int ledBrightness = 0;
int ledCurrentBright = 0;

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 200;           // interval in milliseconds
unsigned long antiShortCicle = 0;        // delay between two separate LED switch


void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Blynk.config(auth);  // in place of Blynk.begin(auth, ssid, pass);
  Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
  while (Blynk.connect() == false) {
    // Wait until connected
  }
  Serial.println("Connected to Blynk server");



  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH); // Swith off the LED at startup
  pinMode(D3, OUTPUT);
  pinMode(D2, INPUT);

  timer.setInterval(50L, DistanceMessure); //timer will run every 50ms
  timer.setInterval(11000L, CheckConnection); // check if still connected every 11 seconds
  timer.setInterval(3000L, HandleClient);

  setupOTA();
  FirstBootUp();
}

BLYNK_WRITE(V0) { // Button Widget function
  //digitalWrite(D1, param.asInt()); // take the state of the Button and send it to the pin

  if (param.asInt() == 0) {
    Blynk.virtualWrite(V1, 0); // Send timer state back to button to update it.
    ledBrightness = 0;
  }
  else {
    Blynk.virtualWrite(V1, 1023); // Send timer state back to button to update it.
    ledBrightness = 1023;
  }
  ledState = param.asInt();
  SwitchLed();
}

BLYNK_WRITE(V1) { // Timer Widget function
  ledBrightness = param.asInt();

  if (ledBrightness > 0) {
    Blynk.virtualWrite(V0, HIGH); // Send timer state back to button to update it.
    ledState = HIGH;
  }
  else {
    Blynk.virtualWrite(V0, LOW); // Send timer state back to button to update it.
    ledState = LOW;
  }
  SwitchLed();
}

BLYNK_WRITE(V3) { // Button Widget function

  int dimmer = param.asInt();
}

void CheckConnection() {
  Connected2Blynk = Blynk.connected();
  if (!Connected2Blynk) {
    Serial.println("Not connected to Blynk server");
    Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
  }
  else {
    Serial.println("Connected to Blynk server");
  }
}

void FirstBootUp() {
  digitalWrite(D1, HIGH); // Swith off the LED at startup
  Blynk.virtualWrite(V0, LOW); // Send ledState button to update it.
  Blynk.virtualWrite(V1, 0); // Send ledState to button to update it.
}

void DistanceMessure() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(D3, LOW);
  delayMicroseconds(5);
  digitalWrite(D3, HIGH);
  delayMicroseconds(10);
  digitalWrite(D3, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(D2, INPUT);
  duration = pulseIn(D2, HIGH);

  // convert the time into a distance
  distance = (duration / 2) / 29.1;

  if (distance < 12) {
    if (antiShortCicle < millis()) {
      previousMillis = millis();
      CheckStatus();
    }
  }
}

void CheckStatus() {
  while (distance < 20) {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you turn ON the LED

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
        ledBrightness = 1023;
        Blynk.virtualWrite(V1, 1023); // Send ledState to button to update it.
        Blynk.virtualWrite(V0, ledState); // Send ledState button to update it.
      } else {
        ledState = LOW;
        ledBrightness = 0;
        Blynk.virtualWrite(V1, 0); // Send ledState to button to update it.
        Blynk.virtualWrite(V0, ledState); // Send ledState button to update it.
      }

      SwitchLed();
      Serial.println(ledState);
      antiShortCicle = millis() + 100;
      return;
    }
  }
}

void SwitchLed() {

  analogWriteFreq(20000);

  if (ledCurrentBright < ledBrightness) { //Check
    for (; ledCurrentBright <= ledBrightness; ledCurrentBright++) {
      analogWrite(D1, cie[ledCurrentBright]); // take the state of the Timer and send it to the pin
      delayMicroseconds(600); // Delay 600 microseconds for smooth operation
    }
  }
  else {
    for (; ledCurrentBright >= ledBrightness; ledCurrentBright--) {
      analogWrite(D1, cie[ledCurrentBright]); // take the state of the Timer and send it to the pin
      delayMicroseconds(600); // Delay 600 microseconds for smooth operation
    }
  }

}

void loop()
{
  Blynk.run();
  timer.run();
}

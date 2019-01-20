/*************************************************************
  Control the LED lamp with Blynk and ultrasonic distance
 *************************************************************/
#define BLYNK_PRINT Serial // Defines the object that is used for printing
//#define BLYNK_DEBUG        // Optional, this enables more detailed prints

#include <ESP8266WiFi.h>
//#include <BlynkSimpleEsp8266_SSL.h> // Before that i use this
#include <BlynkSimpleEsp8266.h>
//#include <ESP8266HTTPClient.h>
//#define BLYNK_DEFAULT_FINGERPRINT "2763955ab22e97f4eb02f55bdb35e9dafae9c97d"
#include <C:\Users\Toni Stoyanov\Desktop\ESP8266_Playground\Kitchen_Light\KitchenLight\OTA.h>

BlynkTimer timer; // Announcing the timer
//bool internalLedState = false;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "1b19d0f5bdc144d7804d6056a0c91d34";
//char server[] = "mrazcom-iot.ddns.net";
//int port = 80;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Viki_Network";
char pass[] = "traxer144";

long duration, distance;
bool ledState = HIGH;
bool firstBoot = true;
int distanceSet = 0;
bool Connected2Blynk = false;

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
  pinMode(D3, OUTPUT);
  pinMode(D2, INPUT);

  timer.setInterval(50L, DistanceMessure); //timer will run every 50ms
  timer.setInterval(11000L, CheckConnection); // check if still connected every 11 seconds

}

BLYNK_WRITE(V0) { // Button Widget function
  digitalWrite(D1, param.asInt()); // take the state of the Button and send it to the pin

  if (param.asInt() == 0) {
    Blynk.virtualWrite(V1, 0); // Send timer state back to button to update it.
  }
  else {
    Blynk.virtualWrite(V1, 1023); // Send timer state back to button to update it.
  }
  ledState = param.asInt();
}

BLYNK_WRITE(V1) { // Timer Widget function
  int dimmer = param.asInt();

  analogWriteFreq(20000);
  analogWrite(D1, dimmer); // take the state of the Timer and send it to the pin

  if (dimmer > 0) {
    Blynk.virtualWrite(V0, HIGH); // Send timer state back to button to update it.
    ledState = HIGH;
  }
  else {
    Blynk.virtualWrite(V0, LOW); // Send timer state back to button to update it.
    ledState = LOW;
  }
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
  digitalWrite(D1, LOW); // Swith off the LED at startup
  Blynk.virtualWrite(V0, LOW); // Send ledState button to update it.
  Blynk.virtualWrite(V1, 0); // Send ledState to button to update it.
  firstBoot = false;
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
      SwitchLed();
    }
  }
}

void SwitchLed() {
  while (distance < 20) {
    
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you turn ON the LED

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
        Blynk.virtualWrite(V1, 1023); // Send ledState to button to update it.
        Blynk.virtualWrite(V0, HIGH); // Send ledState button to update it.
      } else {
        ledState = LOW;
        Blynk.virtualWrite(V1, 0); // Send ledState to button to update it.
        Blynk.virtualWrite(V0, LOW); // Send ledState button to update it.
      }
      digitalWrite(D1, ledState);
        Serial.println(ledState);  
      antiShortCicle = millis() + 200;
      return;
    }
  }
}

void loop()
{
  if (Connected2Blynk) {
    Blynk.run();
  }
  timer.run();
}

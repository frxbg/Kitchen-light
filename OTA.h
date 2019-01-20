#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

String overTheAirURL;


void enterOTA() {

  Serial.println(String("Firmware update URL: ") + overTheAirURL);
  switch (ESPhttpUpdate.update(overTheAirURL)) {
  case HTTP_UPDATE_FAILED:
    Serial.println(String("Firmware update failed (error ") + ESPhttpUpdate.getLastError() + "): " + ESPhttpUpdate.getLastErrorString());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("No firmware updates available.");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("Firmware update: OK.");
    delay(10);
    ESP.restart();
    break;
  }
}

BLYNK_WRITE(InternalPinOTA) {
  overTheAirURL = param.asString();

  // Disconnect, not to interfere with OTA process
  Blynk.disconnect();

  // Start OTA
  enterOTA();  
  delay(500);
}

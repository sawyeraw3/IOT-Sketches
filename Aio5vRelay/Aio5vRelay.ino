#include "config.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <WebOTA.h>
#include <ESP8266WiFi.h>

#define UPDATE_PORT 8080
#define UPDATE_LINK "/update"
#define RELAY_PIN 5
#define ONBOARD_LED 16
#define ESP_ID "esp0"

AdafruitIO_Feed *home_iot = io.feed("home_iot");

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode (ONBOARD_LED, OUTPUT);

  Serial.begin (115200);
  io.connect();
  home_iot->onMessage(handleMessage);
  
  // Wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(10);
  }

  webota.init(UPDATE_PORT, UPDATE_LINK);
  
  // Successfully connected
  Serial.println();
  Serial.println(io.statusText());
  
  String intitializedStatusString = WiFi.localIP().toString() + ":" + UPDATE_PORT + UPDATE_LINK + " to update";
  Serial.println(intitializedStatusString);

  // Set analogWrite range for ESP8266
  #ifdef ESP8266
    analogWriteRange(255);
  #endif
  delay(10);
  // Outlet is connected to NC and powered when relay is off
  // so LED indicates outlet 'on'
  digitalWrite (ONBOARD_LED, 0);

  home_iot->save((String)ESP_ID + " | " + intitializedStatusString);
}

void loop() {
  io.run();
  webota.handle();
}

void handleMessage(AdafruitIO_Data *data) {
  String str = data->toString();
  String targetEsp = str.substring(0, str.indexOf(":"));
  int val = str.substring(str.indexOf(":") + 1).toInt();
  if (targetEsp == ESP_ID) {
    handleRelayMessage(val);
  }
}

void handleRelayMessage(int i) {
  if(i) {
    // Relay turned on, so outlet is 'off'
    digitalWrite (RELAY_PIN, 1);
    digitalWrite (ONBOARD_LED, 1);
  } else {
    digitalWrite (RELAY_PIN, 0);
    digitalWrite (ONBOARD_LED, 0);
  }
}

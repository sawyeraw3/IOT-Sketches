// Simple sketch for controlling a 5v relay attached to esp8266-12E via Adafruit IO
// The relay is powered via the Vin on the esp, and controls a 110v outlet attached to its NC terminal.

#include "config.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define RELAY_PIN 5
#define ONBOARD_LED 16

AdafruitIO_Feed *relay_feed = io.feed("relay");

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode (ONBOARD_LED, OUTPUT);

  Serial.begin (115200);
  io.connect();
  relay_feed->onMessage(handleRelayMessage);
  // Wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(1);
  }
  // Successfully connected
  Serial.println();
  Serial.println(io.statusText());

  // Set analogWrite range for ESP8266
  #ifdef ESP8266
    analogWriteRange(255);
  #endif
  delay(10);
  // Outlet is connected to NC and powered when relay is off
  // so LED indicates outlet 'on'
  digitalWrite (ONBOARD_LED, 0);
}

void loop() {
  // Always listen for MQTT messages from AdafruitIO feeds
  io.run();
}

void handleRelayMessage(AdafruitIO_Data *data) {
  String s = data->toString();
  // Could splice s before analysis
  if(s == "relay:1") {
    // Relay turned on, so outlet is 'off'
    toggle_relay_and_led(1);
  } else if(s == "relay:0") {
    toggle_relay_and_led(0);
  }
}

void toggle_relay_and_led(int i) {
  digitalWrite (RELAY_PIN, i);
  digitalWrite (ONBOARD_LED, i);
}


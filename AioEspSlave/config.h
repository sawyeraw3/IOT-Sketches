#ifndef CONFIG_H
#define CONFIG_H

#ifdef __AVR__
#include <avr/power.h>
#endif

#include <EEPROM.h>

#include <WebOTA.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "AdafruitIO_WiFi.h"
#define IO_USERNAME    ""
#define IO_KEY         ""

#define WIFI_CHAR_ARRAY_SIZE 50
#define WIFI_CRED_ADDR 0
struct {
  char WIFI_SSID[WIFI_CHAR_ARRAY_SIZE] = "";
  char WIFI_PASS[WIFI_CHAR_ARRAY_SIZE] = "";
} wifiCreds;

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, wifiCreds.WIFI_SSID, wifiCreds.WIFI_PASS);
AdafruitIO_Feed *home_iot = io.feed("home_iot");

ESP8266WebServer server(80);
bool connectedToWiFi = true;

#define ESP_ID "esp0"
//TODO eventually, we want multifunctionality
#define ESP_FUNCTION "relay"
#define ESP_AP_SSID "ESP_soft_AP"
#define ESP_AP_PASS "password"
#define CONN_ATTEMPT_DUR 30000
#define UPDATE_PORT 8080
#define UPDATE_LINK "/update"

const char HOME_PAGE_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h2>ESP AP Homepage</h2>
<h3>Enter network credentials to attempt connection</h3>
<form action="/wifi_credentials" autocomplete="off">
  SSID:
  <br>
  <input type="text" name="ssid" value="">
  <br>
  Password:
  <br>
  <input type="text" name="password" value="">
  <br>
  <input type="submit" value="Submit">
</form>
</body>
</html>
)=====";

const char CONN_ATMPT_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h2>Credentials Submitted</h2>
<h3>Attempting Connection to " + server.arg("ssid") + "</h3>
</body>
</html>
)=====";
#endif
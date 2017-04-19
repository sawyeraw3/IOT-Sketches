//Code utilizes two main online examples:
//http://www.instructables.com/id/TESTED-Timekeeping-on-ESP8266-Arduino-Uno-WITHOUT-/
//http://www.esp8266.com/viewtopic.php?f=29&t=6007#p31405
#include "AdafruitIO_WiFi.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Adafruit-IO & WiFi personal information
#define IO_USERNAME    "aio-username"
#define IO_KEY         "aio-key"
#define WIFI_SSID       "your-ssid"
#define WIFI_PASS       "your-pass"
// pin connected to DH11
#define DATA_PIN 5

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
// create DHT11
DHT_Unified dht(DATA_PIN, DHT11);

// set up the 'temperature' and 'humidity' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *logger = io.feed("ranData");

unsigned long timeNow = 0;
unsigned long timeLast = 0;
//Time start Settings:
int startingHour = 0; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int minutes = 0;
int seconds = 0;
int hours = startingHour;
int days = 0;

bool shouldSend = false;

void setup() {
  // start the serial connection
  Serial.begin(115200);  
  Serial.println();
  
  // initialize dht11
  dht.begin();
  
  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    //Serial.print(".");
    delay(1);
  }
  
  // we are connected, uncomment to optain useful information about connectivity
  /*Serial.println();
  Serial.println(io.statusText());
  Serial.println();
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("Current Time");
  */
  
  String cTime = getCurrentTime();
  timeLast = -1 * getSecond(cTime);
  timeLast += (millis()/1000);
  startingHour = getHourEST(cTime);
  minutes = getMinute(cTime);
  hours = startingHour;
}

void loop() {
  io.run();
  if(minutes % 1 == 0 && shouldSend) {
    getTempHumid();
    //Serial.println("cT:" + String(hours) + ":" + String(minutes));
    shouldSend = false;
  }
  
  timeNow = (millis()/1000); // the number of seconds that have passed since boot
  seconds = timeNow - timeLast; //the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds == 60) {
    timeLast = timeNow;
    minutes += 1;
    shouldSend = true;
  } else if (seconds > 60) { // if boot time + retrieved time > 60, next minute
    minutes += (seconds/60); //will happen infrequently, mostly when booting with 
    timeLast = timeNow - (seconds%60); // < 10 seconds left in the current minute
    shouldSend = true;
  }
  
  // If one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes == 60){ 
    minutes = 0;
    hours = hours + 1;
  }
  // If one hour has passed, start counting minutes from zero and add one hour to the clock
  if (hours == 24){
    hours = 0;
    days = days + 1;
  }
  // If 24 hours have passed , add one day
  // Fetch actual time from online to prevent skewed time given from the board
  if (hours ==(24 - startingHour)){
    String cTime = getCurrentTime();
    timeLast = -1 * getSecond(cTime);
    timeLast += (millis()/1000);
    startingHour = getHourEST(cTime);
    minutes = getMinute(cTime);
    hours = startingHour;
  }
}

void getTempHumid() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  humidity->save(event.relative_humidity);
  dht.temperature().getEvent(&event);
  float celsius = event.temperature;
  float fahrenheit = (celsius * 1.8) + 32;
  temperature->save(fahrenheit);
  logger->save("H: " + String(hours) + " M: " + String(minutes));
  delay(5000);
}

int getHourEST(String cTime) {
  String cHour = cTime.substring(cTime.indexOf(":") - 2, cTime.indexOf(":"));
  if(cHour.startsWith("0")) {
    if(cHour.substring(1,2).toInt() < 4) {
      return 24 + (cHour.substring(1,2).toInt() - 4);
    } else {
      return cHour.substring(1,2).toInt() - 4;
    }
  } else {
    return cHour.toInt() - 4;
  }
}

int getMinute(String cTime) {
  String cMinute = cTime.substring(cTime.indexOf(":") + 1, cTime.indexOf(":") + 3);
  if(cMinute.startsWith("0")) {
    return cMinute.substring(1,2).toInt();
  } else {
    return cMinute.toInt();
  }
}

int getSecond(String cTime) {
  String cSecond = cTime.substring(cTime.indexOf(":") + 4, cTime.indexOf(":") + 6);
  if(cSecond.startsWith("0")) {
    return cSecond.substring(1,2).toInt();
  } else {
    return cSecond.toInt();
  }
}

String getCurrentTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    delay(1);
    //Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");
  while(!!!client.available()) {
     delay(1);
  }

  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}

// DotStar matrix script for HUZZAH32
#include "config.h"
#include "heartVars.h"
#include "patternVars.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <Fonts/TomThumb.h>
#include "Arduino.h"

//Declarations for esp32
#define DATAPIN    27
#define CLOCKPIN   13

#define SHIFTDELAY 100

boolean clockMode = false;
unsigned long timeNow = 0;
unsigned long timeLast = 0;
//Time start Settings:
int startingHour = 0; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int minutes = 0;
int seconds = 0;
int hours = startingHour;
int days = 0;

// set up the feeds
AdafruitIO_Feed *msg = io.feed("matrixmessage");
AdafruitIO_Feed *brightness = io.feed("matrixbrightness");
AdafruitIO_Feed *color = io.feed("matrixcolor");

//Matrix declaration
Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
                                  12, 6, DATAPIN, CLOCKPIN,
                                  DS_MATRIX_BOTTOM     + DS_MATRIX_LEFT +
                                  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
                                  DOTSTAR_BGR);

int mesStartPos = matrix.width();
bool mesDisplayed = false;

const uint16_t Colors[] = {
  matrix.Color(255, 0, 0),   //red
  matrix.Color(255, 125, 0), //orange
  matrix.Color(200, 255, 0), //yellowish
  matrix.Color(0, 255, 0),   //green
  matrix.Color(0, 255, 65),
  matrix.Color(65, 255, 0),
  matrix.Color(0, 255, 225), //blue
  matrix.Color(150, 0, 255), //purple
  matrix.Color(255, 0, 220), //pink
  matrix.Color(255, 65, 0),  //reddish
  matrix.Color(255, 220, 0),  //orange/yellow
  matrix.Color(255, 65, 220)
};

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Starting Adafruit IO");
  io.connect();
  
  // set up a message handlers for feeds.
  msg->onMessage(handleStringMessage);
  brightness->onMessage(handleBrightnessMessage);
  color->onMessage(handleColorMessage);
  
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println(io.statusText());
  
  matrix.begin();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);
  int initBrightness = 5;
  matrix.setBrightness(initBrightness);
  brightness->save(initBrightness);
  matrix.show();

  
  msg->save("Boot!");
  Serial.println("Boot!");
  updateTime();
}

void loop() {
  io.run();
  if(heartOn) {
    //Second condition provides for immediate start when 'heart' called
    if(millis() >= heartStart + heartShiftDelay || (cStep == One && heartStart == 0)) {
      heartStart = millis();
      updateHeart(cStep);
    }
  } else if(rainbowOn) {
    //Second condition provides for immediate start when 'heart' called
    if(millis() >= rainbowStart + rainbowShiftDelay || rainbowStart == 0) {
      rainbowStart = millis();
      updateRainbowScroll();
    }
  }
  if(clockMode){
    clockTick();
  }
}



//::AdafruitIO Handers::\\

void handleStringMessage(AdafruitIO_Data *data) {
  heartOn = false;
  rainbowOn = false;
  String s = data->toString();
  String sl = s;
  sl.toLowerCase();
  //Start scrolling the message from off screen right
  mesStartPos = matrix.width();
  if (sl == "heart" || sl == "love") {
    heartOn = true;
     //Reset heartStart so it begins immediately each time it is called
    heartStart = 0;
  } else if(sl == "date" || sl == "time") {
    String cDate = getDateAndTime();
    while(!mesDisplayed) {
      writePhrase(cDate);
    }
  } else if(sl == "rainbow") {
    rainbowOn = true;
    rainbowStart = 0;
    //updateRainbowScroll();
  }else {
    while(!mesDisplayed) {
      writePhrase(s);
    }
  }
  //Clear screen when message has fully scrolled
  if(mesDisplayed) {
    matrix.fillScreen(0);
    mesDisplayed = false;
  }
}

void handleColorMessage(AdafruitIO_Data *data) {
  heartOn = false;
  rainbowOn = false;
  matrix.fillScreen(matrix.Color(data->toRed(), data->toGreen(), data->toBlue()));
  matrix.show();
}

void handleBrightnessMessage(AdafruitIO_Data *data) {
  int b = data->toInt();
  matrix.setBrightness(b);
  matrix.show();
}



//::LED related functions::\\

void writePhrase(String s) {
  //int i = random(sizeof(Colors));
  matrix.fillScreen(0);
  matrix.setCursor(mesStartPos, 5);
  //Print message beggining t the cursor
  for (byte j = 0; j < s.length(); j++) {
    //Issues w/ changing colors on large String
    matrix.setTextColor(Colors[5]); //i%sizeof(Colors)]);
    matrix.print(s[j]);
  }
  
  //Decrement x, set mesDisplayed = true once message has scrolled off screen
  //Message length multipled by 4 = font width + space between letters
  if (--mesStartPos < -float(s.length() * 4)) {
    //mesStartPost gets reset in handler
    //mesStartPos = matrix.width();
    mesDisplayed = true;
  }
  matrix.show();
  delay(SHIFTDELAY);
}

void updateHeart(heartStep hStep) {  //int x, int y) {
  matrix.fillScreen(0);
  //matrix.setCursor(x, y);
  
  //Draw 'i' at startPos
  int startPos = 0;
  matrix.drawPixel(startPos, 5, Colors[6]);
  matrix.drawPixel(startPos, 4, Colors[6]);
  matrix.drawPixel(startPos, 3, Colors[6]);
  matrix.drawPixel(startPos, 1, Colors[6]);

  //Draw 'u'
  int uPos = startPos + 9;
  matrix.drawPixel(uPos, 5, Colors[6]);
  matrix.drawPixel(uPos+1, 5, Colors[6]);
  matrix.drawPixel(uPos+2, 5, Colors[6]);
  matrix.drawPixel(uPos, 4, Colors[6]);
  matrix.drawPixel(uPos+2, 4, Colors[6]);
  matrix.drawPixel(uPos, 3, Colors[6]);
  matrix.drawPixel(uPos+2, 3, Colors[6]);

  //drawHeart in relation to startPos
  int heartPos = startPos + 5;
  
  if(cStep == Three) {
    cStep = One;
    hStepThree(heartPos);
  } else if(cStep == Two) {
    cStep = Three;
    hStepTwo(heartPos);
    //heartOn = false;
  } else if(cStep == One) {
    cStep = Two;
    hStepOne(heartPos);
  }
  matrix.show();
  //Serial.println(cStep);
  delay(5);
}

void hStepOne(int xPos) {
  matrix.drawPixel(xPos, 3, Colors[0]);
  matrix.drawPixel(xPos+1, 2, Colors[0]);
  matrix.drawPixel(xPos-1, 2, Colors[0]);
}

void hStepTwo(int xPos) {
  matrix.drawPixel(xPos, 4, Colors[0]);
  matrix.drawPixel(xPos+1, 3, Colors[0]);
  matrix.drawPixel(xPos-1, 3, Colors[0]);
  matrix.drawPixel(xPos, 2, Colors[0]);
  matrix.drawPixel(xPos+2, 2, Colors[0]);
  matrix.drawPixel(xPos-2, 2, Colors[0]);
  matrix.drawPixel(xPos+1, 1, Colors[0]);
  matrix.drawPixel(xPos-1, 1, Colors[0]);
  matrix.drawPixel(xPos+2, 1, Colors[0]);
  matrix.drawPixel(xPos-2, 1, Colors[0]);
}

void hStepThree(int xPos) {
  matrix.drawPixel(xPos, 5, Colors[0]);
  matrix.drawPixel(xPos+1, 4, Colors[0]);
  matrix.drawPixel(xPos-1, 4, Colors[0]);
  matrix.drawPixel(xPos-2, 3, Colors[0]);
  matrix.drawPixel(xPos+2, 3, Colors[0]);
  matrix.drawPixel(xPos-3, 2, Colors[0]);
  matrix.drawPixel(xPos+3, 2, Colors[0]);
  matrix.drawPixel(xPos, 1, Colors[0]);
  matrix.drawPixel(xPos-3, 1, Colors[0]);
  matrix.drawPixel(xPos+3, 1, Colors[0]);
  matrix.drawPixel(xPos-1, 0, Colors[0]);
  matrix.drawPixel(xPos+1, 0, Colors[0]);
  matrix.drawPixel(xPos-2, 0, Colors[0]);
  matrix.drawPixel(xPos+2, 0, Colors[0]);
}


//::Pattern related functions::\\

void updateRainbowScroll() {
  //matrix.fillScreen(0);
  //Start at bottom left
  matrix.setCursor(mesStartPos, 5);
  //Print message beggining t the cursor
  //for (int i = 1; i < 3; i++) {
  int x = 0;
    //Display diagonal lines of color
    for (int j = 0; j < matrix.width() * 2; j++) {
      for (int k = 0; k < matrix.height(); k++) {
        x++;
        if(x > sizeof(Colors)) {
          x = 0;
        }
        matrix.drawPixel((mesStartPos + j + k), k, Colors[x]);//j, Colors[i]);
      }
    }
  //}

  //Decrement mesStartPos so next render is shifted
  //mesStartPos-=1;
  if (--mesStartPos < -matrix.width() * 2) {
    mesStartPos = matrix.width();
  }
  matrix.show();
  delay(SHIFTDELAY);
}


//::Clock related functions::\\

void clockTick() {
    timeNow = (millis()/1000); // the number of seconds that have passed since boot
    seconds = timeNow - timeLast; //the number of seconds that have passed since the last time 60 seconds was reached.
    if (seconds == 60) {
      timeLast = timeNow;
      minutes += 1;
    } else if (seconds > 60) { // if boot time + retrieved time > 60, increase minute (MAY NOT BE NECESSARY, TEST)
      minutes += (seconds/60); //will happen infrequently, mostly when booting with 
      timeLast = timeNow - (seconds%60); // < 10 seconds left in the current minute
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
      updateTime();
    }
}

void updateTime() {
  String cDate = getDateAndTime();  //String cTime = getTime();
  timeLast = -1 * getSecond(cDate);
  timeLast += (millis()/1000);
  startingHour = getHourEST(cDate);
  minutes = getMinute(cDate);
  hours = startingHour;
}

int getHourEST(String cDate) {
  String cHour = cDate.substring(cDate.indexOf(":") - 2, cDate.indexOf(":"));
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

int getMinute(String cDate) {
  String cMinute = cDate.substring(cDate.indexOf(":") + 1, cDate.indexOf(":") + 3);
  if(cMinute.startsWith("0")) {
    return cMinute.substring(1,2).toInt();
  } else {
    return cMinute.toInt();
  }
}

int getSecond(String cDate) {
  String cSecond = cDate.substring(cDate.indexOf(":") + 4, cDate.indexOf(":") + 6);
  if(cSecond.startsWith("0")) {
    return cSecond.substring(1,2).toInt();
  } else {
    return cSecond.toInt();
  }
}

String getTime() {
  //cut up getDateAndTime result for just time
}

String getDate() {
  //cut up getDateAndTime result for just date
}

String getDateAndTime() {
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

#include "WiFi.h"

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
                                  
//Matrix declaration
Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
                                  12, 6, DATAPIN, CLOCKPIN,
                                  DS_MATRIX_BOTTOM     + DS_MATRIX_LEFT +
                                  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
                                  DOTSTAR_BGR);

const uint16_t Colors[] = {
  matrix.Color(255, 0, 0),   //red
  matrix.Color(255, 125, 0), //orange
  matrix.Color(200, 255, 0), //yellowish
  matrix.Color(0, 255, 0),   //green
  matrix.Color(0, 255, 225), //blue
  matrix.Color(150, 0, 255), //purple
  matrix.Color(255, 0, 220), //pink
  matrix.Color(255, 65, 0),  //reddish
  matrix.Color(255, 220, 0),  //orange/yellow
  matrix.Color(255, 255, 255),  //orange/yellow
  matrix.Color(0, 0, 0)  //orange/yellow
};

const char* ssid     = "";
const char* password = "";
bool f = false;
int mWidth = matrix.width();
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  matrix.begin();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(millis() > 10000) {
      //Serial.print("startAP");
      startAP();
    }
  }
  
  String st = "boot";
  handleStringMessage(st);
  

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  server.begin();
  //startAP();
}

void loop() {
  // put your main code here, to run repeatedly:
  runServer();
}

void runServer() {
  
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        //Serial.print("this" + c);
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn off.<br>");
            client.print("Click <a href=\"/Hr\">here</a> to run heart.<br>");
            
            
            
            client.println("<!DOCTYPE HTML>");
            client.println("<HTML>");
            client.print("<HEAD>");
            client.print("<TITLE> Test</title>");
            client.print("</head>");
            client.println("<BODY>");
            client.println("<FORM ACTION='/' method=get >"); //uses IP/port of web page
            client.println("Brightness: <INPUT TYPE=TEXT NAME='BRIGHTNESS' VALUE='' SIZE='25' MAXLENGTH='50'><BR>");
            client.println("<INPUT TYPE=SUBMIT NAME='submit' VALUE='Change Brightness'>");
            client.println("</FORM>");
            client.print("<br>");
            client.print("</BODY>");
            client.println("</HTML>");
            

            
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          on();               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          off();                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /Hr")) {          
          //patternOn = true;
          drawHeart();
        }
        if (currentLine.endsWith("Brightness")) {//("GET /?BRIGHTNESS=&submit=Change+Brightness")) {
          int b = currentLine.substring(currentLine.indexOf("=") + 1, currentLine.indexOf("&")).toInt();
          if(b > 0 && b < 240) {
            currentLine = "";
            matrix.setBrightness(b);
            matrix.show();
          } else {
            String cLine = currentLine.substring(currentLine.indexOf("=") + 1, currentLine.indexOf("&"));
            currentLine = "";
            handleStringMessage(cLine);
          }
          client.stop();
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void startAP() {
  
  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  
  Serial.println("Waiting for SmartConfig.");

  String st = "SmartConfig";
  handleStringMessage(st);
  handleStringMessage(st);
  
  //Wait for SmartConfig packet from mobile
  
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  delay(10);
  /*
  WiFi.softAP("esp32", "dotstarmatrix");
  Serial.print("Connecting to ");
  Serial.print("esp32"); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  */
}

void off(){
  matrix.fillScreen(Colors[10]);
  matrix.show();
}

void on(){
  matrix.fillScreen(Colors[9]);
  matrix.show();
}

void handleStringMessage(String s) {
  //s.toLowerCase();
  if (s == "heart" || s == "love") {
    drawHeart();
  } else if(s == "date" || s == "time") {
    //String cDate = getDateAndTime();
    while(!f) {
      //writePhrase(cDate);
    }
  } else {
    while(!f) {
      writePhrase(s);
    }
  }

  if(f) {
    matrix.fillScreen(0);
    f = false;
  }
}

void writePhrase(String s) {
  //int i = random(sizeof(Colors));
  matrix.fillScreen(0);
  matrix.setCursor(mWidth, 5);
  for (byte j = 0; j < s.length(); j++) {
    //Issues w/ changing colors on large String
    matrix.setTextColor(Colors[5]); //i%sizeof(Colors)]);
    matrix.print(s[j]);
  }
  if (--mWidth < -float(s.length() * 4)) {
    mWidth = matrix.width();
    f = true;
  }
  matrix.show();
  delay(SHIFTDELAY);
}

void drawHeart() {  //int x, int y) {
  for(int i = 0; i<3; i++) {
  matrix.fillScreen(0);
  //matrix.setCursor(x, y);
  int x = 6;
    
  //Draw 'i'
  matrix.drawPixel(11, 0, Colors[6]);
  matrix.drawPixel(11, 1, Colors[6]);
  matrix.drawPixel(11, 2, Colors[6]);
  matrix.drawPixel(11, 4, Colors[6]);

  //Draw 'u'
  matrix.drawPixel(0, 0, Colors[6]);
  matrix.drawPixel(1, 0, Colors[6]);
  matrix.drawPixel(2, 0, Colors[6]);
  matrix.drawPixel(0, 1, Colors[6]);
  matrix.drawPixel(2, 1, Colors[6]);
  matrix.drawPixel(0, 2, Colors[6]);
  matrix.drawPixel(2, 2, Colors[6]);
  
  matrix.drawPixel(6, 3, Colors[0]);
  matrix.show();
  delay(500);
  
  matrix.drawPixel(6, 1, Colors[0]);
  matrix.drawPixel(6, 2, Colors[0]);
  matrix.drawPixel(5, 2, Colors[0]);
  matrix.drawPixel(7, 2, Colors[0]);
  matrix.drawPixel(6, 3, Colors[0]);
  matrix.drawPixel(7, 3, Colors[0]);
  matrix.drawPixel(5, 3, Colors[0]);
  matrix.drawPixel(8, 3, Colors[0]);
  matrix.drawPixel(4, 3, Colors[0]);
  matrix.drawPixel(7, 4, Colors[0]);
  matrix.drawPixel(8, 4, Colors[0]);
  matrix.drawPixel(5, 4, Colors[0]);
  matrix.drawPixel(4, 4, Colors[0]);
  matrix.show();
  delay(500);
  
  //given initial pixel, draw heart?
  matrix.drawPixel(x, 0, Colors[0]);
  matrix.drawPixel(x+1, 1, Colors[0]);
  matrix.drawPixel(x-1, 1, Colors[0]);
  matrix.drawPixel(x-2, 2, Colors[0]);
  matrix.drawPixel(x+2, 2, Colors[0]);
  matrix.drawPixel(x-3, 3, Colors[0]);
  matrix.drawPixel(x+3, 3, Colors[0]);
  matrix.drawPixel(x, 4, Colors[0]);
  matrix.drawPixel(x-3, 4, Colors[0]);
  matrix.drawPixel(x+3, 4, Colors[0]);
  matrix.drawPixel(x-1, 5, Colors[0]);
  matrix.drawPixel(x+1, 5, Colors[0]);
  matrix.drawPixel(x-2, 5, Colors[0]);
  matrix.drawPixel(x+2, 5, Colors[0]);
  matrix.show();
  delay(500);
  }
  delay(1);
}

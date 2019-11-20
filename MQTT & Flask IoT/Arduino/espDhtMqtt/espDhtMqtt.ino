#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"

#define MQTT_CHANNEL "ESP8266Client"
#define MQTT_USERNAME "MQTT_USERNAME"
#define MQTT_PASSWORD "MQTT_PASSWORD"
#define LAN_SSID "ssid"
#define LAN_PASSWORD "password"
#define MQTT_SERVER "mqtt.server.ip.value"
#define DHTTYPE DHT11
#define DHT_MQTT_CHANNEL "/esp8266/dhtdata"
#define time_between_messages 30000
#define ESP_DEVICE_NAME "esp_dht11_1"

const int DHTPin = 14;
DHT dht(DHTPin, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

long now = millis();
long lastMeasure = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(LAN_SSID);
  WiFi.begin(LAN_SSID, LAN_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());
}

// On Publish callback for subscribed channels, if desired
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print("| Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
  return;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CHANNEL, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" delaying 2s");
      delay(2000);
    }
  }
}

void setup() {
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop()){
    client.connect(MQTT_CHANNEL, MQTT_USERNAME, MQTT_PASSWORD);
  }
  now = millis();
  if (now - lastMeasure > time_between_messages) {
    lastMeasure = millis();
    // Exclude 'true' in dht.readTemperature() call to get Celsius temp value
    float h = dht.readHumidity();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(f)) {
      Serial.println("DHT sensor reading failed.");
      return;
    }
    
    // Add 'false' as 3rd argument in dht.computeHeadIndex() to compute temperature in Celsius
    float deg_f = dht.computeHeatIndex(f, h);
    static char readingTemp[7];
    dtostrf(deg_f, 6, 2, readingTemp);
    
    static char readingHumid[7];
    dtostrf(h, 6, 2, readingHumid);

    char dataToPublish[80];
    String dhtReadings = "{ \"temperature\": \"" + String(readingTemp) + "\", \"humidity\" : \"" + String(readingHumid) + "\", \"device\" : \"" + ESP_DEVICE_NAME + "\"}";
    dhtReadings.toCharArray(dataToPublish, (dhtReadings.length() + 1));
    
    // Publish data
    client.publish(DHT_MQTT_CHANNEL, dataToPublish);
    Serial.println("Data published");
    Serial.println(dataToPublish);
  }
}

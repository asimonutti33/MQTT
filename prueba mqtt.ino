#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include "DHT.h"
//#include <Adafruit_Sensor.h>
//#include <DHT.h>
//#include <DHT_U.h> 
//replace with your network credentials
#define WIFI_SSID "Nadadewifigratis 2.4GHz"
#define WIFI_PASSWORD "00428574198"

// Node Red on Windows Mosquitto MQTT Broker
//#define MQTT_HOST IPAddress(192, 168, 29, 33)
// For a cloud MQTT broker, type the domain name
#define MQTT_HOST "broker.mqtt-dashboard.com"
#define MQTT_PORT 1883


//MQTT Topics
#define MQTT_PUB_TEMP "esp8266/dhtReadmqttdata/temperature"
#define MQTT_PUB_HUM "esp8266/dhtReadmqttdata/humidity"

// Digital pin connected to the DHT sensor
#define DHTPIN 0

// Uncomment whatever DHT sensor type you're using
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21 // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

float temp;
float hum;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0; 
const long interval = 5000;

void connectToWifi() {
Serial.println("Connecting to Wi-Fi...");
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
Serial.println("Connected to Wi-Fi.");
connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
Serial.println("Disconnected from Wi-Fi.");
mqttReconnectTimer.detach(); 
wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
Serial.println("Connecting to MQTT...");
mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
Serial.println("Connected to MQTT.");
Serial.print("Session present: ");
Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
Serial.println("Disconnected from MQTT.");

if (WiFi.isConnected()) {
mqttReconnectTimer.once(2, connectToMqtt);
}
}

void onMqttPublish(uint16_t packetId) {
Serial.print("Publish acknowledged.");
Serial.print(" packetId: ");
Serial.println(packetId);
}

void setup() {
Serial.begin(115200);
Serial.println();

dht.begin();

wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);


mqttClient.onConnect(onMqttConnect);
mqttClient.onDisconnect(onMqttDisconnect);
mqttClient.onPublish(onMqttPublish);
mqttClient.setServer(MQTT_HOST, MQTT_PORT);
// If your broker requires authentication (username and password), set them below
//mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
connectToWifi();
}

void loop() {
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
previousMillis = currentMillis;
hum = dht.readHumidity();
temp = dht.readTemperature();

if (isnan(temp) || isnan(hum)) {
Serial.println(F("Failed to read from DHT sensor!"));
return;
} 

// Publish an MQTT message on topic esp8266/dht/temperature
uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str()); 
Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
Serial.printf("Message: %.2f \n", temp);

// Publish an MQTT message on topic esp8266/dht/humidity
uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str()); 
Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
Serial.printf("Message: %.2f \n", hum);
}
}
/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include "analogWrite.h"
//#include <Wire.h>
//#include <Adafruit_BME280.h>
//#include <Adafruit_Sensor.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

bool bomb_state = false;

String timertext;
int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


// Replace the next variables with your SSID/Password combination
const char* ssid = "HGG Private";
const char* password = "HGGamespass10!";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.1.101";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int notConnected = 0; //number of attempts before restarting ESP32

//uncomment the following lines if you're using SPI
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

/*
Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
float temperature = 0;
float humidity = 0;
*/
// LED Pin
const int ledPin = 18;

void setup() {
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  /*if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }*/
  //delay(3000);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  matrix.fillScreen(LOW);
  matrix.write();
  matrix.setIntensity(1); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  

  //analogWriteResolution(ledPin, 10);
  //pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(30);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    notConnected++;
    delay(500);
    Serial.print(".");
    if(notConnected > 3){
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  /*if (String(topic) == "Candymonium Desk Timer") {
    if(messageTemp == "on"){
    bomb_state = true; }
    if(messageTemp == "off"){
    bomb_state = false;
    matrix.fillScreen(LOW);
    matrix.write();
    }}*/
  if (String(topic) == "Candymonium RPi Desk Timer") {
    timertext = messageTemp;
    matrix.fillScreen(LOW);
    matrix.drawChar(2, 1, timertext[3], HIGH, LOW, 1);
    matrix.drawChar(9, 1, timertext[4], HIGH, LOW, 1);
    matrix.drawChar(14, 1, timertext[5], HIGH, LOW, 1);
    matrix.drawChar(19, 1, timertext[6], HIGH, LOW, 1);
    matrix.drawChar(26, 1, timertext[7], HIGH, LOW, 1);
    matrix.write();
    }
  }


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Candymonium ESP32 Entryway Timer")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("Candymonium RPi Desk Timer");
      //client.subscribe("bomb_prop/timer");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Temperature in Celsius
    //temperature = bme.readTemperature();   
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
    /*char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    humidity = bme.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);*/
  }
}

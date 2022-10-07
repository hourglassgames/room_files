
int relayPin[3] = {5,18,19};
int groundPins[2] = {26,22};
int purplePin = 23;
int bluePin = 27;
int purplePinStatus;
int bluePinStatus;
int audioStatus = 0;
/*
 * MQTT Initial Code
 */

#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "HGG Private";
const char* password = "HGGamespass10!";

// Add your MQTT Broker IP address
const char* mqtt_server = "192.168.1.101";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int value = 0;
const char* device = "Candymonium ESP32 Compartments";//what you want to call this device
String device_name = device;
String msg_string;

void subscriptions(){ //add all desired topics to receive and post to in this function
  client.subscribe("My_Topic");
  client.subscribe("device_test");
  client.subscribe("compartments");
  client.subscribe("Candymonium Audio Buttons");
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempts++;
    if(attempts > 3){
      ESP.restart();}
    
    Serial.print(".");
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
    messageTemp += (char)message[i];}
  Serial.println();

  /* 
   *  This is your message handling for incoming messages.  
   *  if(String(topic) == "desired_topic"){Action}
   *    if(messageTemp == "desired_message"){Action}
   *  Send messages out with:
   *    client.publish("topic", "message");
   */
  if (String(topic) == "My_Topic") {
    Serial.print("My_Topic Received");
    if(messageTemp == "message"){
      client.publish("My_Topic", "Message Received at ESP32");
      //Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
    }
    
  }
  if (String(topic) == "device_test"){
    Serial.println("device_test received");
    if(messageTemp == "test"){
      msg_string = device_name + " is OK";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      }
    }
  if (String(topic) == "compartments"){
    if (messageTemp == "one"){
      //turn on one for 500 ms
      digitalWrite(relayPin[0], LOW);
      delay(500);
      digitalWrite(relayPin[0], HIGH);
    }
    if ((messageTemp == "one_auto") && (audioStatus == 0)){
      //turn on one for 500 ms
      digitalWrite(relayPin[0], LOW);
      delay(500);
      digitalWrite(relayPin[0], HIGH);
      audioStatus = 1;
    }
    if (messageTemp == "two"){
      //turn on two for 500ms
      digitalWrite(relayPin[1], LOW);
      delay(500);
      digitalWrite(relayPin[1], HIGH);
    }
    if (messageTemp == "three"){
      //turn on three for 500ms
      digitalWrite(relayPin[2], LOW);
      delay(500);
      digitalWrite(relayPin[2], HIGH);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(device)) {
      Serial.println("connected");
      // Subscribe to as many topics as needed
      subscriptions();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/*
 * End of MQTT initial code
 */



void setup() {
  Serial.begin(115200);
  
  /*
   * MQTT setup code
   */
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  /*
   * End of MQTT setup code
   */

for (int i = 0; i < 3; i++){
  pinMode(relayPin[i],OUTPUT);
  digitalWrite(relayPin[i], HIGH);
}
for (int i = 0;i < 2; i++){
  pinMode(groundPins[i], OUTPUT);
  digitalWrite(groundPins[i],LOW);
}
 pinMode(purplePin, INPUT_PULLUP);
 pinMode(bluePin, INPUT_PULLUP);
}


void loop() {
  /*
   * MQTT loop code
   */
  purplePinStatus = digitalRead(purplePin);
  if (purplePinStatus == LOW){
    client.publish("Candymonium Audio Buttons", "purple");
    delay(500);
  }
  bluePinStatus = digitalRead(bluePin);
  if (bluePinStatus == LOW){
    client.publish("Candymonium Audio Buttons", "blue");
    delay(500);
  }
  
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  
/*
 * End of MQTT loop code
 */
}

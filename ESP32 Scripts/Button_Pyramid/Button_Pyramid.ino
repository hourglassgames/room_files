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
const char* device = "Room 1 Button Pyramid";//what you want to call this device
String device_name = device;
String msg_string;

void subscriptions(){ //add all desired topics to receive and post to in this function
  client.subscribe("My_Topic");
  client.subscribe("device_test");
  client.subscribe("Button Pyramid");
  client.subscribe("Closet");
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
  if (String(topic) == "Button Pyramid") {
    Serial.print("Button Pyramid Received");
    if(messageTemp == "desk"){
      client.publish("My_Topic", "Message Received at ESP32");
      //Serial.println("on");
    }
    else if(messageTemp == "closet"){
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
}

void reconnect() {
  // Loop until we're reconnected
  //delay(2000);
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
      delay(4000);
    }
  }
  
}


/*
 * End of MQTT initial code
 */

int switchPins[6] = {21,19,18,5,23,22};//32,33,34,35,36,3932,33,34,35,36,39
int lightPins[6] = {12,13,14,25,26,27};
int pinVal;
int solution1[6] = {0,5,2,4,1,3};
int solution2[6] = {1,2,4,3,0,5};
int counter1 = 0;
int counter2 = 0;
int relayPins[2] = {32,33};



void setup() {
  // put your setup code here, to run once:

    /*
   * MQTT setup code
   */
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  /*
   * End of MQTT setup code
   */
  
  for(int i = 0; i < 6;i++){
    pinMode(switchPins[i],INPUT_PULLUP);
    pinMode(lightPins[i],OUTPUT);
    digitalWrite(lightPins[i],LOW);
}
  for (int i = 0; i < 2; i++){
    pinMode(relayPins[i],OUTPUT);
    digitalWrite(relayPins[i],HIGH);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
    for(int i = 0; i < 6; i++){
      pinVal = digitalRead(switchPins[i]);
      if (pinVal == LOW){
        if (solution1[counter1] == i){
          digitalWrite(lightPins[i], HIGH);
          delay(50);
          digitalWrite(lightPins[i],LOW);
          counter1 = counter1 + 1;

          if(counter1 == 6){
            for (int i = 0; i < 6; i++){
              digitalWrite(lightPins[i],HIGH);
              delay(50);
              digitalWrite(lightPins[i],LOW);
              delay(50);
              digitalWrite(relayPins[0], LOW);
              
              
            }
            if (!client.connected()) {
            reconnect();
          }
           
           client.publish("Closet", "open");
            
          }
        }
        else if(solution1[counter1] != i){
          for (int j = 0; j < 6; j++){
            digitalWrite(lightPins[j],LOW);
            counter1 = 0;
          }
        }
        
    }
    
  }
  for(int i = 0; i < 6; i++){
      pinVal = digitalRead(switchPins[i]);
      if (pinVal == LOW){
        if (solution2[counter2] == i){
          digitalWrite(lightPins[i], HIGH);
          counter2 = counter2 + 1;
          delay(50);
          digitalWrite(lightPins[i],LOW);

          if(counter2 == 6){
            for (int i = 0; i < 6; i++){
              digitalWrite(lightPins[i],HIGH);
              delay(50);
              digitalWrite(lightPins[i],LOW);
              delay(50);
              digitalWrite(relayPins[0], LOW);
              
              
            }
            if (!client.connected()) {
            reconnect();
          }
           
           client.publish("Bait and Switch Desk", "opendesk");
            
          }
        }
        else if(solution2[counter2] != i){
          for (int j = 0; j < 6; j++){
            digitalWrite(lightPins[j],LOW);
            counter2 = 0;
          }
        }
        
    }
    
    }
  
  
  delay(150);
  
}

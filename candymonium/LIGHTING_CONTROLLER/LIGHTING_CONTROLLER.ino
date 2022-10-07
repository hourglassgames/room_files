

int relayPins[4] = {5,18,19,21};

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
const char* device = "Candymonium ESP32 Lighting Controller";//what you want to call this device
String device_name = device;
String msg_string;

void subscriptions(){ //add all desired topics to receive and post to in this function
  client.subscribe("My_Topic");
  client.subscribe("device_test");
  client.subscribe("Candymonium ESP32 Lights");
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
    if(messageTemp == "reset"){
      msg_string = device_name + " is resetting";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      for (int i = 0; i < 4; i++){
        digitalWrite(relayPins[i],HIGH);
      }
      }
    }
  if (String(topic) == "Candymonium ESP32 Lights"){
    Serial.println("Lights request");
    if(messageTemp == "blacklight_on"){
      msg_string = "blacklight turned on";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[3],LOW);
      }
    if(messageTemp == "blacklight_off"){
      msg_string = "blacklight turned off";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[3],HIGH);
      }
    if(messageTemp == "entryway_on"){
      msg_string = "entryway turned on";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[2],HIGH);
      }
    if(messageTemp == "entryway_off"){
      msg_string = "entryway turned off";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[2],LOW);
      }
     if(messageTemp == "research_on"){
      msg_string = "research turned on";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[1],HIGH);
      }
    if(messageTemp == "research_off"){
      msg_string = "research turned off";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[1],LOW);
      }
    if(messageTemp == "kitchen_on"){
      msg_string = "kitchen turned on";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[0],HIGH);
      }
    if(messageTemp == "kitchen_off"){
      msg_string = "kitchen turned off";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      digitalWrite(relayPins[0],LOW);
      }
     if(messageTemp == "all_lights_on"){
      msg_string = "all_lights turned on";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      for (int i = 0; i < 3; i++){
        digitalWrite(relayPins[i],HIGH);
      }
        digitalWrite(relayPins[3], LOW);
      }
    if(messageTemp == "all_lights_off"){
      msg_string = "all_lights turned off";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      for (int i = 0; i < 3; i++){
        digitalWrite(relayPins[i],LOW);
      }
      digitalWrite(relayPins[3], HIGH);
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

   for (int i = 0; i < 4; i++){
    pinMode(relayPins[i],OUTPUT);
    digitalWrite(relayPins[i],HIGH);
   }
   
}


void loop() {
  /*
   * MQTT loop code
   */
  
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
/*
 * End of MQTT loop code
 */
}

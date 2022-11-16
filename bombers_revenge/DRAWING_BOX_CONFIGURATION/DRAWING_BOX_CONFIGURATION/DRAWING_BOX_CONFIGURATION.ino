/*
 * MQTT Initial Code
 */

#include <WiFi.h>
#include <PubSubClient.h>

int panelPin = 2;

// Replace the next variables with your SSID/Password combination
const char* ssid = "HGG Private";
const char* password = "HGGamespass10!";

// Add your MQTT Broker IP address
const char* mqtt_server = "192.168.1.101";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int value = 0;
const char* device = "Bait and Switch Closet";//what you want to call this device
String device_name = device;
String msg_string;

void subscriptions(){ //add all desired topics to receive and post to in this function
  client.subscribe("Closet");
  client.subscribe("device_test");
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
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
  if (String(topic) == "Closet") {
    Serial.print("Closet Received");
    if(messageTemp == "open"){
      //Serial.println("on");
      digitalWrite(panelPin, LOW);
      delay(500);
      digitalWrite(panelPin, HIGH);
      client.publish("Closet", "Closet Open");
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
   pinMode(panelPin, OUTPUT);
   digitalWrite(panelPin, HIGH);
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

/*
 * Start of MQTT code
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
const char* device = "Candymonium ESP32 Logic Puzzle";//what you want to call this device
String device_name = device;
String msg_string;

void subscriptions(){
  client.subscribe("My_Topic");
  client.subscribe("device_test");
  client.subscribe("Candymonium Logic Puzzle");
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
 * End of MQTT Code
 */

/* 
 *  MUX code
 *  
 */

//MUX control pins for first 15 inputs
int s0 = 12;
int s1 = 14;
int s2 = 27;
int s3 = 26;

//Mux in "SIG" pin for first 15 inputs
int SIG_pin = 32;
int SIG_pin2 = 33;

int readMux(int channel, int pin){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(pin);
  //int val = digitalRead(pin);

  //return the value
  return val;
}
/*
 * End of MUX Code
 */

void setup() {
  Serial.begin(9600);
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  
  /*
   * MQTT Setup Code
   */
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  /*
   * End of MQTT Setup Code
   */

  //pinMode(SIG_pin,INPUT_PULLUP);
  //pinMode(SIG_pin2, INPUT_PULLUP);
  

   
}

void loop() {
  /*
  * MUX loop
  */
  char count[2];
  int correct_count = 0;
  //String 
  for(int i = 0; i < 15; i ++){
    /*Serial.print(i);
    Serial.print(": ");
    Serial.print(readMux(i, SIG_pin));
    Serial.print(", ");*/
    delay(10)   ;
    if(readMux(i, SIG_pin) < 200){
    //if(readMux(i, SIG_pin) == 0){
      correct_count++;
    }
    }
  for(int i = 0; i < 10; i ++){
    /*Serial.print(i);
    Serial.print(": ");
    Serial.print(readMux(i, SIG_pin2));
    Serial.print(", ");*/
    delay(10)   ;
    if(readMux(i, SIG_pin2) < 200){
    //if(readMux(i, SIG_pin2) == 0){
      correct_count++;
    }
    }


    
   itoa(correct_count,count,10);
   client.publish("Candymonium Logic Puzzle",count);
   delay(50);
   Serial.println("");
  

  /*
   * End of MUX loop
   */
  
  
  
  
  /*
   * MQTT Loop
   */
  
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
/*
 * End of MQTT loop
 */
 
}

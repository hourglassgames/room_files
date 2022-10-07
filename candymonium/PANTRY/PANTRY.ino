/*
 * Code block for MQTT communication
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
const char* device = "Candymonium ESP32 Pantry";//what you want to call this device
String device_name = device;
String msg_string;


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

void subscriptions(){
  client.subscribe("My_Topic");
  client.subscribe("device_test");
  client.subscribe("Candymonium Pantry Code");
  client.subscribe("Candymonium Pantry Status");
  client.subscribe("Candymonium Audio Buttons");
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
    if (messageTemp == "reset"){
      msg_string = device_name + " is restarting";
      msg_string.toCharArray(msg,50);
      client.publish("device_test",msg);
      delay(100);
      ESP.restart();
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
 * End of MQTT code block, some additional code in setup and loop functions
 * 
 */




#include "Adafruit_Keypad.h"
int ledPin[3] = {22,4,5};
int mosfetPin[2] = {15,23};
int groundPin[2] = {19,23};

const byte ROWS = 4; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33,32}; //connect to the column pinouts of the keypad

int codeCount = 0;

char* master_1 = "72179941";
char* master_2 = "4321";
char* master_3 = "1357";
char current_entry[9];

int master_solved[3] = {0,0,0};

int orangeButton = 18;
int orange_button_status;

//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  customKeypad.begin();
for (int i = 0; i < 3; i++){
  pinMode(ledPin[i], OUTPUT);}
for (int i = 0; i < 2; i++){
  pinMode(mosfetPin[i], OUTPUT);
  digitalWrite(mosfetPin[i], LOW);
}
for (int i = 0; i < 2; i++){
 pinMode(groundPin[i], OUTPUT);
 digitalWrite(groundPin[i], LOW);
}
 pinMode(orangeButton, INPUT_PULLUP);
 
/*
 * Setup instructions for MQTT
 */
 setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  
  // put your main code here, to run repeatedly:
  customKeypad.tick();
    if (!client.connected()) {
    reconnect();
      }
    orange_button_status = digitalRead(orangeButton);
    if (orange_button_status == LOW){
      client.publish("Candymonium Audio Buttons","orange");
      delay(500);
    }
  //while(customKeypad.available()){
    keypadEvent e = customKeypad.read();
     /*
      * MQTT loop code
      */

    //client.loop();

    //Serial.print((char)e.bit.KEY);
    if(e.bit.EVENT == KEY_JUST_PRESSED){
      current_entry[codeCount] = (char)e.bit.KEY;
      Serial.println(current_entry);
      client.publish("Candymonium Entryway Code",current_entry);
      //Serial.println(master_1);
      codeCount++;
      blinkLights();
      
      }
    //Serial.println(" pressed");
    //else if(e.bit.EVENT == KEY_JUST_RELEASED) Serial.println(" released");

  if (codeCount == 8){
    if(!strcmp(master_1,current_entry)){
      Serial.println("1st password matches"); 
      //master_solved[0] = 1;
      digitalWrite(mosfetPin[0], HIGH);
      delay(500);
      digitalWrite(mosfetPin[0],LOW);
      //master_solved[0] = 0;
      msg_string = device_name + " is unlocked";
      msg_string.toCharArray(msg,50);
      client.publish("Candymonium Pantry Code",msg);
      clearData();
      }
      /*
    else if (!strcmp(master_2,current_entry)){
      Serial.println("2nd password matches");
      clearData();
      master_solved[1] = 1;
    }
    else if (!strcmp(master_3,current_entry)){
      Serial.println("3rd password matches");
      clearData();
      master_solved[2] = 1;
    }*/
    else {
      Serial.println("Incorrect Password");
      blinkLights();
      delay(100);
      blinkLights();
      delay(100);
      blinkLights();
      delay(100);
      clearData();
    }
  
    
  }
  for (int i=0;i < 3; i++){
    if (master_solved[i] == 1){
      digitalWrite(ledPin[i], HIGH);}
    else {
      digitalWrite(ledPin[i], LOW);}
  delay(10);
  char solved_status[3];
  for (int i = 0; i < 3; i++){
    solved_status[i] = char(master_solved[i]);
  }
  //String solved_status = master_solved[0] + master_solved[1] + master_solved[2];
  //msg_string = master_solved[0] + master_solved[1] + master_solved[2];
  //msg_string.toCharArray(msg,50);
  //client.publish("Candymonium Pantry Status",msg);
  client.loop();
  }
/*
  if (master_solved[0] == 1){ //&& (master_solved[1] == 1) && (master_solved[2] == 1)){
    digitalWrite(mosfetPin[0], HIGH);
    delay(2000);
    digitalWrite(mosfetPin[0],LOW);
    master_solved[0] = 0;
    msg_string = device_name + " is unlocked";
    msg_string.toCharArray(msg,50);
    client.publish("Candymonium Pantry Code",msg);*/
    /*
    while (true){
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
    }*/
  }




void clearData(){
  while (codeCount != 0){
    current_entry[codeCount--] = 0;
  //Serial.println(current_entry);
  }
}

void blinkLights(){
  for (int i = 0; i < 3; i++){
    digitalWrite(ledPin[i], HIGH);}
  delay(100);
  for (int i = 0; i < 3; i++){
    digitalWrite(ledPin[i], LOW);}
}

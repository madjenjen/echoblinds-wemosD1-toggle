/*
  Control servo motors through a relay server
*/

#include <ArduinoJson.h>
#include "config.h"
#include <Servo.h>
#include <ESP8266WiFi.h>

Servo servo;
int fractionOpen = -1;
WiFiClient client;

void setup()
{
  servo.attach(4);
  Serial.begin(9600);
  Serial.println("Initiating.");
  delay(1000);

  // try to connect to wifi
  WiFi.begin(NETID, PASS);

  for (int i=0; i<10;i++){
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to WiFi.");
      delay(500);
      break;
    }

    if (WiFi.status() != WL_CONNECTED){
      Serial.println("Couldn't connect to WiFi. Stand by.");
    }
    delay(5000);
  }
}

void loop()
{
  sendGET();
  unsigned int i = 0; //timeout counter
  int n = 1; // char counter
  char json[100]="{";
  
  while (!client.available() && i<10) {
    Serial.println("Client is not available.");
    delay (5000);
    i++;
  }

  i = 0;

  while (!client.find("{")) {
    if (client.read()==-1){
      break;
    }
  }

  //Read JSON file.
  while (i<150) {
    if (client.available()) {
      char c = client.read();
      json[n]=c;
      n++;
      i=0;
    }
    i++;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  boolean success_ok = root["success"];


  //When message is available, toggle the motors.
  if(success_ok) {
    int microseconds;
    if (fractionOpen < 0 || fractionOpen == 0) {
      Serial.println("Opening blinds.");
      microseconds = 1700;
      servo.writeMicroseconds(microseconds);  
      delay(8000);
      servo.writeMicroseconds(1500); 
      fractionOpen = 1;
    }
    else if (fractionOpen == 1) {
      Serial.println("Closing blinds.");
      microseconds = 1300;
      servo.writeMicroseconds(microseconds);  
      delay(8000);
      servo.writeMicroseconds(1500); 
      fractionOpen = 0;
    }
  }
  delay(5000);
}

//Send web request.
void sendGET() {
  client.connect(DST_IP, PORT);
  if (client.connected()) {
    Serial.println("Connected to server.");
    String cmd = "GET /api/get?apiKey=";
    cmd += API_KEY;
    cmd += "&clientId=";
    cmd += CLIENT_ID;
    cmd += " HTTP/1.0";
    Serial.println(cmd);
    client.println(cmd);
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("Failed to connect to server. Stand by.");
    Serial.println();
  }
}

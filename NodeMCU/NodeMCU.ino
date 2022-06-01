#include <FS.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>

#include<SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define ALIAS "pieled"
#define LED D0 

SoftwareSerial NodeSerial(D0, D1); // RX | TX

// PM2.5 Sensor PIN
int PMMeasurePin = A0;
int PMLedPowerPin = D2;

// WIFI
const char* ap_name = "ESP8266-Safely-Live-Your-Life-AP";

// connnect to netpie
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "a8dfc6a7-1799-46a1-b799-55735a2003ff";
const char* mqtt_username = "hM5wWXzpJjzEmp42M4FKcYbjWw7hqfaX";
const char* mqtt_password = "3ThdRrqDFw5nrm4WH*OmsDhxNMJJiUxP";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

StaticJsonDocument<200> data;
int stateLed = 0;

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (mqttClient.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      mqttClient.subscribe("@msg/led");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
//  Serial.println(message);

  if(message == "on" ){
    stateLed = 1;
    digitalWrite(LED, LOW);
  }
  else if(message == "off") {
    stateLed = 0;
    digitalWrite(LED, HIGH);
  }
  //Serial.println(stateLed==1? "1":"0");
}

void uploadDataToNetPie(StaticJsonDocument<200> data) {
  char buffer[256];
  if (mqttClient.connected()) {
    mqttClient.loop();
    StaticJsonDocument<200> netpieData;
    netpieData["data"] = data;
    serializeJson(netpieData, buffer);
    mqttClient.publish("@shadow/data/update", buffer);
  } else reconnect();
}

void saveData(String e) {
  StaticJsonDocument<200> temp;
  deserializeJson(temp, e);
  data["temp"] = temp[0];
  data["humi"] = temp[1];
  data["lumi"] = temp[2];
  stateLed = temp[3];
  data["led"] = stateLed;
//  data["pm"] = temp[3];
}

float getDustDensity() {
  digitalWrite(PMLedPowerPin, LOW);
  delayMicroseconds(280);
  float voMeasured = analogRead(PMMeasurePin); // 0-1023
  delayMicroseconds(40);
  digitalWrite(PMLedPowerPin, HIGH);
  delayMicroseconds(9680);

  float calcVoltage = voMeasured * (3.3 / 1024); // 0.0-3.3

  float dustDensity = 0.17 * calcVoltage - 0.1;

  return dustDensity;
}

// Led switch

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  NodeSerial.begin(115200);
  
  // init PM sensor
  pinMode(PMLedPowerPin, OUTPUT);

  // led
  pinMode(LED, OUTPUT);

  // init WIFI
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(ap_name)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  // init mqtt
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
}


unsigned long uploadDataPreviousMillis = 0;
const long uploadDataInterval = 1000;

unsigned long readPMPreviousMillis = 0;
const long readPMInterval = 1000;

void loop() {
//  if (NodeSerial.available()) {
//    String data_str = NodeSerial.readString();
//    Serial.print("[NodeMCU] Receive data: ");
//    Serial.print(data_str);
//    Serial.println("");
//    if (data_str.startsWith("[")) {
//      saveData(data_str);
//    }
//  }
  if (Serial.available()) {
    String data_str = Serial.readString();
    Serial.print("r");
    Serial.println(stateLed);
//    Serial.print("[NodeMCU] Receive data: ");
//    Serial.print(data_str);
//    Serial.println(""); //receive data;
    if (data_str.startsWith("[")) {
      saveData(data_str);
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - uploadDataPreviousMillis >= uploadDataInterval) {
    uploadDataPreviousMillis = currentMillis;
    uploadDataToNetPie(data);
  }

  if (currentMillis - readPMPreviousMillis >= readPMInterval) {
    readPMPreviousMillis = currentMillis;
    float dustDensity = getDustDensity();
    data["pm"] = dustDensity;
    //data["led"] = stateLed;
  }
}

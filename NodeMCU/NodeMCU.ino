#include <FS.h>

#include <PubSubClient.h>

#include <ArduinoJson.h>

#include<SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

SoftwareSerial NodeSerial(D0, D1); // RX | TX

// WIFI
const char* ap_name = "ESP8266-Safely-Live-Your-Life-AP";

// connnect to netpie
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "a8dfc6a7-1799-46a1-b799-55735a2003ff";
const char* mqtt_username = "hM5wWXzpJjzEmp42M4FKcYbjWw7hqfaX";
const char* mqtt_password = "3ThdRrqDFw5nrm4WH*OmsDhxNMJJiUxP";

unsigned long previousMillis = 0;
const long interval = 1000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

StaticJsonDocument<200> data;

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (mqttClient.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      //      mqttClient.subscribe("@msg/led");
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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);

  if (String(topic) == "") {}
}

void sendDataToNetPie(StaticJsonDocument<200> data) {
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
  data["pm"] = temp[3];
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting...");

  // init
  pinMode(D0, INPUT);
  pinMode(D1, OUTPUT);
  NodeSerial.begin(115200);

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

void loop() {
  while (NodeSerial.available()) {
    String data_str = NodeSerial.readString();
    if (data_str.startsWith("[")) {
      Serial.print("[NodeMCU] Receive data: ");
      Serial.print(data_str);
      Serial.println("");
      saveData(data_str);
    }
  }

  while (Serial.available()) {
    String data_str = Serial.readString();
    if (data_str.startsWith("[")) {
      Serial.print("[NodeMCU] Receive data: ");
      Serial.print(data_str);
      Serial.println("");
      saveData(data_str);
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendDataToNetPie(data);
  }
}

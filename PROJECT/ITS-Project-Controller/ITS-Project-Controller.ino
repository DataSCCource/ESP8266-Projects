/*
 Smarthub Controller.
 This program controls incoming sensors and an ws2812b LED strip
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


const char* ssid = "SmartHub";
const char* password = "Lismaholed";
const char* mqtt_server = "192.168.0.10";

WiFiClient espClient;
PubSubClient client(espClient);

String currentMode = "motion";
String currentColor = "#FF0000";
String currentBrightness = "100";
bool motionDetected = false;
int ledOntimeDuration = 15; // seconds
int ledOntimeSince = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  handleLight();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      client.subscribe("sensor/#");
      client.subscribe("actor/#");
      client.subscribe("control/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  //get rid of leftovers in the payload-buffer
  payload[length] = '\0';
  String value = (char*)payload;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strcmp(topic, "sensor/motion/1") == 0) {
    // 1 -> true; 0 -> false
    setMotionDetected(strcmp(value, "1") == 0);
  } else if(strcmp(topic, "sensor/distance/1") == 0) {
    setDistance(value);

  } else if(strcmp(topic, "control/led/mode") == 0) {
    setWorkingMode(value);
  } else if(strcmp(topic, "control/led/color") == 0) {
    setWorkingMode("color");
    setLedColor(value);   
  } else if(strcmp(topic, "control/led/brightness") == 0) {
    setLedBrightness(value);
  } else {
    Serial.println("## Unbehandeltes Topic: " + value);
  }
}

void setWorkingMode(String newMode) {
  Serial.println("Setting mode to " + newMode);
  currentMode = newMode;

  handleLight();
}

void setLedColor(String newColor) {
  Serial.println("Setting color to " + newColor);
  currentColor = newColor;

  handleLight();
}

void setLedBrightness(String newBrightness) {
  Serial.println("Setting brightness to " + newBrightness);
  currentBrightness = newBrightness;

  handleLight();
}

void setMotionDetected(bool newDetected) {
  if(newDetected) {
    // resetTimer
    ledOntimeSince = millis();  
  }
  
  motionDetected = newDetected
  handleLight();
  
}

void setDistance(String newDistance) {
  Serial.println("Setting distande to " + newDistance);

  // TODO set distance
  handleLight();
}

// Handle LED with current settings
void handleLight() {

  if (strcmp(currentMode, "color") == 0){
    client.publish("actor/led/color", currentColor);

  } else if (strcmp(currentMode, "motion") == 0){
    if(motionDetected) {
      client.publish("actor/led/enable", "1");
    }

  } else if (strcmp(currentMode, "blink") == 0){
    client.publish("actor/led/mode", "blink");

  } else if (strcmp(currentMode, "fade") == 0){
    client.publish("actor/led/mode", "fade");

  } else if (strcmp(currentMode, "fire") == 0){
     client.publish("actor/led/mode", "fire");

  } else {
    client.publish("actor/led/enable", "0");
  }

  
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if((ledOntimeSince + ledOntimeDuration*1000) > millis() && strcmp(currentMode, "motion") == 0) {
    client.publish("actor/led/enable", "0");
  }
}

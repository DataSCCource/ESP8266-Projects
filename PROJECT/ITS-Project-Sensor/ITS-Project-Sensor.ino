/*
 LismahoLED Sensor.
 This program uses sensors-values to detect motion, brightness and distance
 The values are sent to the central Hub
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_VL53L0X.h"

const char* ssid = "SmartHub";
const char* password = "Lismaholed";
const char* mqtt_server = "192.168.0.10";

String SENSOR_NR = "1";
String MQTT_TOPIC = "/sensor/motion/"+SENSOR_NR;
String MQTT_TOPIC_CONTROL = "/control/sensor/motion";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define LED_STATUS  2  // D4
#define MOTION_PIN  16 // D0

bool state = LOW;
bool stateLowSent = false;
bool stateHighSent = false;

int maxBrightness = 512; // max 1023
int currentDistance = 0;
int lastDistance = 0;
bool distanceEnabled = true;

bool sensorEnabled = true;

// Main setup function
void setup() {
  Serial.begin(115200);
  pinMode(MOTION_PIN, INPUT);
  pinMode(LED_STATUS, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
}

// Main loop function
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(sensorEnabled) {
    if(isDarkEnough()) {
      detectedMotion();
      String tmpTopic = MQTT_TOPIC_CONTROL+"/detected";
      
      if(state == HIGH && !stateHighSent) {
        client.publish(tmpTopic.c_str(), "1");
        stateHighSent = true;
        stateLowSent = false;
      } else if (state == LOW && !stateLowSent) {
        client.publish(tmpTopic.c_str(), "0");
        stateLowSent = true;
        stateHighSent = false;
      }
    }

    if(distanceEnabled) {
      measureDistance();
      if(abs(currentDistance - lastDistance) > 25) {
        lastDistance = currentDistance;
        String tmpTopic = MQTT_TOPIC_CONTROL+"/distance";
        String distanceString = (String) currentDistance;
        Serial.println(distanceString);
        client.publish(tmpTopic.c_str(), distanceString.c_str());
      }
    }    
  }

  delay(100);
}

// Setup / connect to Wifi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_STATUS, LOW); 
    delay(250);
    digitalWrite(LED_STATUS, HIGH); 
    delay(250);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// (Re-)Connect to MQTT-Broker and subscribe to sensor topic
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(LED_STATUS, LOW); 
    delay(500);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      digitalWrite(LED_STATUS, HIGH); 

      String subTopic = MQTT_TOPIC+"/#";
      sendStatus();
      client.subscribe(subTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// publish current status via MQTT 
void sendStatus() {
  Serial.println("## Sending status");

  String tmpTopic = MQTT_TOPIC+"/maxbrightness";
  String maxBrightString = (String)maxBrightness;
  client.publish(tmpTopic.c_str(), maxBrightString.c_str());
}

// Callback method when messages for subscribed topic are comming in.
void callback(char* topic_char, byte* payload, unsigned int length) {
  //get rid of leftovers in the payload-buffer
  payload[length] = '\0';
  String value = (char*)payload;
  String topic = (String) topic_char;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(topic.equals(MQTT_TOPIC+"/enable")) {
    setEnable(value);
  } else if(topic.equals(MQTT_TOPIC+"/maxbrightness")) {
    setMaxBrightness(value);
  } else {
    Serial.println("## Unbehandeltes Topic: " + value);
  }
}

// 0/1 for enabling/disabling the sensor unit
void setEnable(String value) {
    if(value.equals("1")) {
      Serial.println("### Enabling Sensor...");
      sensorEnabled = true;
    } else {
      Serial.println("### Disabling Sensor...");
      sensorEnabled = false;
    }
}

// Set the max brightness (0-1024)
void setMaxBrightness(String value) {
  maxBrightness = value.toInt();
}

// Use maxBrightness to determine if it is too bright for the motion sensor
bool isDarkEnough() {
  int sensorValue = analogRead(A0);   // read the input on analog pin 0
  //Serial.print((String)sensorValue + " | Dark enough: ");   // print out the value you read
  //Serial.println(sensorValue < maxBrightness?"TRUE":"FALSE");
  if(sensorValue < maxBrightness) {
    return true;
  } else {
    return false;
  }
}

// Detect motion
bool detectedMotion() {
  int val = digitalRead(MOTION_PIN);   // read sensor value
  if (val == HIGH) {           // check if the sensor is HIGH
    if (state == LOW) {
      Serial.println("Motion detected!"); 
      state = HIGH;       // update variable state to HIGH
      return true;
    }
  } else {
      if (state == HIGH){
        Serial.println("Motion stopped!");
        state = LOW;       // update variable state to LOW
        return false;
    }
  }
}

// Use VL53L0X to determine distance
void measureDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  if (measure.RangeStatus != 4) { // phase failures have incorrect data
    currentDistance = measure.RangeMilliMeter;
  }
}

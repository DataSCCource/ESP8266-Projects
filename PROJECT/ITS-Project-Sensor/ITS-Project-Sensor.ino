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

int sensor = 16; // D0
int led = LED_BUILTIN;

bool state = LOW;
bool stateLowSent = false;
bool stateHighSent = false;


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(sensor, INPUT);
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

      //client.subscribe("sensor/#");
      //client.subscribe("actor/#");
      //client.subscribe("control/#");
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
  } else if(strcmp(topic, "sensor/distance/1") == 0) {
  } else if(strcmp(topic, "control/led/mode") == 0) {
  } else if(strcmp(topic, "control/led/color") == 0) {
  } else {
    Serial.println("## Unbehandeltes Topic: " + value);
  }
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  motionDetected();
  if(state == HIGH && !stateHighSent) {
    client.publish("sensor/motion", "1");
    stateHighSent = true;
    stateLowSent = false;
  } else if (state == LOW && !stateLowSent) {
    client.publish("sensor/motion", "0");
    stateLowSent = true;
    stateHighSent = false;
  }
}

bool motionDetected() {
  int val = digitalRead(sensor);   // read sensor value
  if (val == HIGH) {           // check if the sensor is HIGH
    digitalWrite(led, HIGH);   // turn LED ON
    delay(100);                // delay 100 milliseconds 
    
    if (state == LOW) {
      Serial.println("Motion detected!"); 
      state = HIGH;       // update variable state to HIGH
      return true;
    }
  } else {
      digitalWrite(led, LOW); // turn LED OFF
      delay(200);             // delay 200 milliseconds 
      
      if (state == HIGH){
        Serial.println("Motion stopped!");
        state = LOW;       // update variable state to LOW
        return false;
    }
  }
}

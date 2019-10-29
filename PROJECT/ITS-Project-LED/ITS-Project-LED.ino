/*
 Smarthub Controller.
 This program controls incoming sensors and an ws2812b LED strip
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#define LED_PIN     14
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

const char* ssid = "SmartHub";
const char* password = "Lismaholed";
const char* mqtt_server = "192.168.0.10";

WiFiClient espClient;
PubSubClient client(espClient);

String currentMode = "color";
String currentColor = "#2836F4";
String currentBrightness = "100";
int ledOntimeDuration = 15; // seconds
int ledOntimeSince = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
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

      client.subscribe("actor/#");
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
  const char* value = (char*)payload;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strcmp(topic, "actor/led/enable") == 0) {
    // 1 -> true; 0 -> false
    if(strcmp(value, "1") == 0) {
      setWorkingMode("color");
    } else {
      setWorkingMode("off");
    }
  } else if(strcmp(topic, "actor/led/color") == 0) {
    setWorkingMode("color");
    setColor(value);

  } else {
    Serial.println("## Unbehandeltes Topic: " + (String)value);
  }
}

void setWorkingMode(const char* newMode) {
  Serial.println("Setting mode to " + (String) newMode);
  currentMode = newMode;

  handleLight();
}

void setColor(const char* newColor) {
  Serial.println("Setting color to " + (String) newColor);
  currentColor = newColor;

  handleLight();
}

void setLedBrightness(String newBrightness) {
  Serial.println("Setting brightness to " + newBrightness);
  currentBrightness = newBrightness;

  handleLight();
}


// Handle LED with current settings
void handleLight() {

  if (currentMode.equals("color")){
    long number = (long) strtol( &currentColor[1], NULL, 16);
    int red = number >> 16 & 0xFF;
    int green = number >> 8 & 0xFF;
    int blue = number & 0xFF;
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB ( red, green, blue);
    }
    FastLED.show();
  } else if (currentMode.equals("off")){
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB ( 0, 0, 0);
    }
    FastLED.show();
  } else {
  }

  
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

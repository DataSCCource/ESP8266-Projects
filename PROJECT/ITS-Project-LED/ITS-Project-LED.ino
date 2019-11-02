/*
 Smarthub Controller.
 This program controls incoming sensors and an ws2812b LED strip
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <FastLED.h>
#include <WS2812FX.h>

#define LED_PIN     14  // D5
#define NUM_LEDS    60

const char* ssid = "SmartHub";
const char* password = "Lismaholed";
const char* mqtt_server = "192.168.0.10";

WiFiClient espClient;
PubSubClient client(espClient);

int MODE_FIREPLACE =   1;
int MODE_COLOR =       2;
int MODE_COLORCHANGE = 3;
int MODE_RAINBOW =     4;
int MODE_FADE =        5;

bool fadeUp = false;
bool fadeDown = false;
int currentBrightness = 0;
long nextFade = 0;

int currentMode = MODE_COLOR;
uint32_t currentColor = RED;
int targetBrightness = 255;
bool directModeSet = false;

WS2812FX ws2812fx = WS2812FX(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ws2812fx.init();
  ws2812fx.setBrightness(0);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(currentColor);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

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
    digitalWrite(LED_BUILTIN, LOW); 
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(250);
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
    delay(100);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      digitalWrite(LED_BUILTIN, LOW); 

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
      setLedBrightness(255);
    } else {
      setLedBrightness(0);
    }
  } else if(strcmp(topic, "actor/led/color") == 0) {
    setLedColor(value);
  } else if(strcmp(topic, "actor/led/mode") == 0) {
    Serial.println("Mode: " + (String)value);
    String newMode = (String)value;
    if(newMode.equals("color")) {
      setWorkingMode(MODE_COLOR);
    } else if(newMode.equals("motion")) {
      setWorkingMode(MODE_COLORCHANGE);
    } else if(newMode.equals("fade")) {
      setWorkingMode(MODE_FADE);
    } else if(newMode.equals("rainbow")) {
      setWorkingMode(MODE_RAINBOW);
    } else if(newMode.equals( "fire")) {
      setWorkingMode(MODE_FIREPLACE);
    } else {
      setWorkingMode(MODE_COLOR);
    }
    
    
  } else if(strcmp(topic, "actor/led/modevalue") == 0) {
    directModeSet = true;
    ws2812fx.setMode(atol(value));
  } else if(strcmp(topic, "actor/led/brightness") == 0) {
    setLedBrightness(atol(value));
  } else {
    Serial.println("## Unbehandeltes Topic: " + (String)value);
  }
}
void setWorkingMode(int newMode) {
  Serial.println("Setting mode to " + (String) newMode);
  currentMode = newMode;
  directModeSet = false;

  handleLight();
}

void setLedColor(const char*  newColor) {
  Serial.println("Setting color to " + (String) newColor);
  currentColor = (uint32_t) strtol( &newColor[1], NULL, 16);
  Serial.println("Setting color to " + (String) currentColor);

  handleLight();
}

void setLedBrightness(int newBrightness) {
  Serial.println("Setting brightness to " + (String) newBrightness);
  targetBrightness = newBrightness;
  fadeUp =  currentBrightness < targetBrightness;
  fadeDown = currentBrightness > targetBrightness;
}


// Handle LED with current settings
void handleLight() {
  ws2812fx.setColor(currentColor);
  if(!directModeSet) {
    if (currentMode == MODE_COLOR) {
      ws2812fx.setMode(FX_MODE_STATIC);
    } else if (currentMode == MODE_FADE ){
      ws2812fx.setMode(FX_MODE_FADE);
    } else if (currentMode == MODE_FIREPLACE ){
      // TODO
    } else if (currentMode == MODE_COLORCHANGE ){
      ws2812fx.setMode(FX_MODE_FIREWORKS);
    } else if (currentMode == MODE_RAINBOW ){
      ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    } else {
      //TODO
    }
  }

 
}


void loop() {
  ws2812fx.service();

  if((fadeUp || fadeDown) && (millis()%5 == 0)) {
    handleFade();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}

void handleFade() {
  if(fadeUp) {
    if(currentBrightness < targetBrightness) {
      currentBrightness++;
      ws2812fx.setBrightness(currentBrightness);
    } else {
      fadeUp = false;
    }
  }

  if(fadeDown) {
    if(currentBrightness > targetBrightness) {
      currentBrightness--;
      ws2812fx.setBrightness(currentBrightness);
    } else {
      fadeDown = false;
    }
  }
}

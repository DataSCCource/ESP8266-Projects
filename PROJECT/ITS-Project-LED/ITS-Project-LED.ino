/*
 Smarthub Controller.
 This program controls incoming sensors and an ws2812b LED strip
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <WS2812FX.h>

#define LED_PIN     14  // D5
#define NUM_LEDS    60

const char* ssid = "SmartHub";
const char* password = "Lismaholed";
const char* mqtt_server = "192.168.0.10";


int MODE_FIREPLACE = -1;

bool fadeUp = false;
bool fadeDown = false;
int currentBrightness = 0;
long nextFade = 0;

int currentMode = 0;
uint32_t currentColor = RED;
int targetBrightness = 255;
int currentSpeed = 1000;

String ACTOR_NR = "1";
String MQTT_TOPIC = "/actor/led/"+ACTOR_NR;

WiFiClient espClient;
PubSubClient client(espClient);
WS2812FX ws2812fx = WS2812FX(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ws2812fx.init();
  ws2812fx.setBrightness(0);
  ws2812fx.setSpeed(currentSpeed);
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
    delay(500);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LEDActor1-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      digitalWrite(LED_BUILTIN, LOW); 

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

void sendStatus() {
  Serial.println("## Sending status");

  String tmpTopic = MQTT_TOPIC+"/enable";
  client.publish(tmpTopic.c_str(), currentBrightness>0?"1":"0");

  tmpTopic = MQTT_TOPIC+"/color";
  String hexColor = "#" + String(currentColor, HEX);
  client.publish(tmpTopic.c_str(), hexColor.c_str());

  tmpTopic = MQTT_TOPIC+"/mode";
  String modeString = (String) currentMode;
  client.publish(tmpTopic.c_str(), modeString.c_str());

  tmpTopic = MQTT_TOPIC+"/brightness";
  String brightString = (String)targetBrightness;
  client.publish(tmpTopic.c_str(), brightString.c_str());

  tmpTopic = MQTT_TOPIC+"/speed";
  String speedString = (String)currentSpeed;
  client.publish(tmpTopic.c_str(), speedString.c_str());

}

void callback(char* topic_char, byte* payload, unsigned int length) {
  //get rid of leftovers in the payload-buffer
  payload[length] = '\0';
  const char* value = (char*)payload;
  String topic = (String) topic_char;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(topic.equals(MQTT_TOPIC+"/enable")) {
    // 1 -> true; 0 -> false
    if(strcmp(value, "1") == 0) {
      setLedBrightness(255);
    } else {
      setLedBrightness(0);
    }
  } else if(topic.equals(MQTT_TOPIC+"/color")) {
    setLedColor(value);
  } else if(topic.equals(MQTT_TOPIC+"/mode")) {
    int newMode = ((String)value).toInt();
    String modeName = "Fire";
    if(newMode >= 0) modeName = _names[newMode];
    Serial.println("Mode: " + modeName);

    setWorkingMode(newMode);
  } else if(topic.equals(MQTT_TOPIC+"/brightness")) {
    setLedBrightness(atol(value));
  } else if(topic.equals(MQTT_TOPIC+"/speed")) {
    setEffectSpeed(atol(value));
  } else {
    Serial.println("## Unbehandeltes Topic: " + (String)value);
    sendStatus();
  }
}


void setEffectSpeed(int newSpeed) {
  currentSpeed = newSpeed;
  ws2812fx.setSpeed(currentSpeed);
}

void setWorkingMode(int newMode) {
  Serial.println("Setting mode to " + (String) newMode);
  currentMode = newMode;

  handleLight();
}

void setLedColor(const char*  newColor) {
  Serial.println("Setting color to " + (String) newColor);
  currentColor = (uint32_t) strtol( &newColor[1], NULL, 16);

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
  if(currentMode == MODE_FIREPLACE) {
    // TODO FIRE ;)
//    ws2812fx.setColor(255, 69,3);
//    ws2812fx.setSpeed(70);
//    ws2812fx.setMode(FX_MODE_FIRE_FLICKER_INTENSE);
      ws2812fx.setMode(FX_MODE_CUSTOM);
      ws2812fx.setCustomMode(customFireEffect);
  } else {
    ws2812fx.setColor(currentColor);
    ws2812fx.setSpeed(currentSpeed);
    ws2812fx.setMode(currentMode);
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

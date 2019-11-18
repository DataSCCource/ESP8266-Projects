/*
 Basic ESP8266 MQTT example
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "<<insertWifiSSID>>";
const char* password = "<<insertWifiPassword>";
const char* mqtt_server = "192.168.4.1";

#define LED1 5
#define LED2 4

WiFiClient espClient;
PubSubClient client(espClient);

int current_brightness = 0;
int current_ct = 0;
int target_brightness = 0;
int target_ct = 50;
float step_brightness = 0.0;
float step_ct = 0.0;
const int MAX_VAL = 1023;
int fadesteps = 0;

// Normalization table to map 0-100 % brightness to corresponding PWM value.
// Brightness perception is not linear but logarithmic. This value table counters that.
const  int pwmtable100[101] = {
  0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 12, 13, 14, 16, 18, 20, 21, 24, 26, 28, 31, 33,
  36, 39, 42, 45, 49, 52, 56, 60, 64, 68, 72, 77, 82, 87, 92, 98, 103, 109, 115, 
  121, 128, 135, 142, 149, 156, 164, 172, 180, 188, 197, 206, 215, 225, 235, 
  245, 255, 266, 276, 288, 299, 311, 323, 336, 348, 361, 375, 388, 402, 417, 
  432, 447, 462, 478, 494, 510, 527, 544, 562, 580, 598, 617, 636, 655, 675, 
  696, 716, 737, 759, 781, 803, 826, 849, 872, 896, 921, 946, 971, 997, 1023 
};

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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT message received on subscribed topics
void callback(char* topic, byte* payload, unsigned int length) {
  //get rid of leftovers in the payload-buffer
  payload[length] = '\0';
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strcmp(topic, "control/ledpanel/status") == 0) {
    Serial.println("## Status");
    
    char cstr[16];
    client.publish("control/ledpanel/brightness", itoa(current_brightness, cstr, 10));
    client.publish("control/ledpanel/colortemperature", itoa(current_ct, cstr, 10));
  } else if (strcmp(topic, "control/ledpanel/fade") == 0){
    String values = (char*)payload;
    Serial.println(values);
    if(values.indexOf('+') != -1) {
      int val0 = values.substring(0,values.indexOf('+')).toInt(); 
      target_brightness = limitInt(val0, 0, 100);
      int val1 = values.substring(values.indexOf('+')).toInt(); 
      target_ct = limitInt(val1, 0, 100);
    } else {
      target_brightness = limitInt(values.toInt(), 0, 100);
    }
    fadeTo();

  } else if (strcmp(topic, "control/ledpanel/fadebrightness") == 0){
    String value = (char*)payload;
    setBrightness(limitInt(value.toInt(), 0, 100));
  } else if (strcmp(topic, "control/ledpanel/fadect") == 0){
    String value = (char*)payload;
    setColorTemperature(limitInt(value.toInt(), 0, 100));
  } else {
    String value = (char*)topic;
    Serial.println("## Unbehandeltes Topic: " + value);
  }
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
      // Once connected, publish an announcement...
      setBrightness(50);

      client.publish("control/ledpanel/status", "OK");
      // ... and resubscribe
      client.subscribe("control/ledpanel/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED1, OUTPUT);     
  pinMode(LED2, OUTPUT);     
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

int lastFade = 0;
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(fadesteps!=0 && lastFade!= millis() && ((millis()%25)==0)) {
    lastFade = millis();
    fadesteps--;

    doStep();
    if(fadesteps == 0) {
      client.publish("control/ledpanel/status", "0");
    }

  }
}

void setBrightness (int new_brightness) {
  target_brightness = new_brightness;
  fadeTo();
}

void setColorTemperature(int new_color_temperature) {
  target_ct = new_color_temperature;
  fadeTo();
}


void fadeTo() {
  int brightness_dist = abs(current_brightness - target_brightness);
  int ct_dist = abs(current_ct - target_ct);
  fadesteps = brightness_dist>ct_dist?brightness_dist:ct_dist;

  step_brightness = (float)(current_brightness - target_brightness)/fadesteps;
  step_ct = (float)(current_ct - target_ct)/fadesteps;
}

void doStep() {
  current_brightness = target_brightness + (int)(step_brightness*fadesteps);
  current_ct = target_ct + (int)(step_ct*fadesteps);

  int tmp_current_value = pwmtable100[current_brightness];

  int value_cw = tmp_current_value * (float)current_ct/100.0;
  int value_ww = tmp_current_value - value_cw;

  analogWrite(LED1, value_ww);
  analogWrite(LED2, value_cw);
}

int limitInt(int value, int min, int max) {
  if(value < min) value = min;
  if(value > max) value = max;
  return value;
}

int indexOfArray(int value) {
  for (int i = 0; i < sizeof pwmtable100/sizeof pwmtable100[0]; i++) {
    if(value == pwmtable100[i]) return i;
    if(pwmtable100[i]> value) return i-1;
  }
}

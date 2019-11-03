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

int currentMode = 0;
uint32_t currentColor = 0xFF0000;
String currentBrightness = "100";
int ledOntimeDuration = 15; // seconds
int ledOntimeSince = 0;
bool motionModeActive = false;
bool motionLedTurnedOff = false;

String ACTORS[] = {"1"};
String SENSORS[] = {"1"};

String ACTOR_TOPIC = "/actor/led";
String SENSOR_TOPIC = "/sensor/motion";
String CONTROL_TOPIC = "/control";


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(motionModeActive && (ledOntimeSince + ledOntimeDuration*1000) < millis() && !motionLedTurnedOff) {
    publishActors("enable", "0");
    motionLedTurnedOff = true;
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
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
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      String subSensor = SENSOR_TOPIC + "/#";
      client.subscribe(subSensor.c_str());

      String subActor = ACTOR_TOPIC + "/#";
      client.subscribe(subActor.c_str());

      String subControl = CONTROL_TOPIC + "/#";
      client.subscribe(subControl.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

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

  if(topic.equals(CONTROL_TOPIC + SENSOR_TOPIC + "/detected")) {
    // 1 -> true; 0 -> false
    setMotionDetected(value.equals("1"));
  } else if(topic.equals(CONTROL_TOPIC + SENSOR_TOPIC + "/enable")) {
    publishSensors("enable", value);
  } else if(topic.equals(CONTROL_TOPIC + SENSOR_TOPIC + "/distance")) {
    setSensorDistance(value);
  } else if(topic.equals(CONTROL_TOPIC + SENSOR_TOPIC + "/maxbrightness")) {
    publishSensors("maxbrightness", value);
  } else if(topic.equals(CONTROL_TOPIC + SENSOR_TOPIC + "/timeout")) {
    ledOntimeDuration = value.toInt();
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/enable")) {
    publishActors("enable", value);
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/color")) {
    publishActors("color", value);
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/mode")) {
    if(value.equals("-2")) {
      publishActors("mode", "0");
      publishActors("enable", "0");
      motionModeActive = true;
    } else {
      publishActors("mode", value);
      motionModeActive = false;
    }
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/brightness")) {
    publishActors("brightness", value);
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/speed")) {
    publishActors("speed", value);
  } else if(topic.equals(CONTROL_TOPIC + ACTOR_TOPIC + "/status")) {
    publishActors("status", value);
  } else {
    Serial.println("## Unbehandeltes Topic: " + value);
  }
}

void publishActors(String topicEnd, String payload) {
    for(int i=0; i<sizeof ACTORS/sizeof ACTORS[0]; i++) {
      String tmpTopic = ACTOR_TOPIC + "/" + ACTORS[i] + "/" + topicEnd;
      client.publish(tmpTopic.c_str(), payload.c_str());
    }
}

void publishSensors(String topicEnd, String payload) {
    for(int i=0; i<sizeof SENSORS/sizeof SENSORS[0]; i++) {
      String tmpTopic = SENSOR_TOPIC + "/" + SENSORS[i] + "/" + topicEnd;
      client.publish(tmpTopic.c_str(), payload.c_str());
    }
}


//----

void setMotionDetected(bool newDetected) {
  if(newDetected) {
    publishActors("enable", "1");

    // resetTimer
    ledOntimeSince = millis();  
    motionLedTurnedOff = false;
  }
}

void setSensorDistance(String newDistance) {
  if(motionModeActive) {
    int bright = 255-newDistance.toInt()/4;
    String brightString = (String)bright;
    
    Serial.println("Setting distande to " + newDistance + " | " + brightString);
    publishActors("brightness", brightString.c_str());
  }

}

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "ESP8266-ap"
#define APPSK  ""
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

bool ledON = false;
bool blinkLed = false;
unsigned long lastSwitch;
ESP8266WebServer server(80);

/* Go to http://192.168.4.1 in a web browser */

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  // setup wifi accesspoint
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // listen to URLs
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/an", handleOn);
  server.on("/aus", handleOff);
  server.on("/blinken", handleBlink);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // if blink is activated, toggle LED every 500ms
  if(blinkLed && (millis()-lastSwitch > 500)) {
    lastSwitch = millis();
    toggleLed();
  } 
  server.handleClient();
}


// deliver website with two buttons if root URL is called ( http://192.168.4.1/ )
void handleRoot() {
  String html ="<!doctype html> <html> <head> <meta charset=\"utf-8\"> <title>LED Toggle</title> <script> function toggleButton() { var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"/toggle\", true); xhttp.send(); } function blinkButton() { var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"/blinken\", true); xhttp.send(); }</script> <noscript> Sie haben JavaScript deaktiviert. </noscript> </head> <body> <h1 style=\"text-align: center\">Toggle LED :)</h1> <div style=\"text-align: center; \"> <button style=\"height:600px; width:600px; font-size:100px\" type=\"button\" onclick=\"toggleButton()\">>Toggle<</button> <button style=\"height:600px; width:600px; font-size:100px\" type=\"button\" onclick=\"blinkButton()\">>blink<</button> </div> </body> </html>";
  server.send(200, "text/html", html);
  Serial.println("Root Document requested");
}

// handler method to turn the led on
void handleOn() {
  blinkLed = false;
  turnLedOn();
}

// turn LED on
void turnLedOn() {
  digitalWrite(LED_BUILTIN, LOW);
  ledON = true;
  server.send(200, "text/html", "<h1>LED Turned ON</h1>");
  Serial.println("LED turned ON");
}

// handler method to turn the led off
void handleOff() {
  blinkLed = false;
  turnLedOff();
}

// turn LED off
void turnLedOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  ledON = false;
  server.send(200, "text/html", "<h1>LED Turned OFF</h1>");
  Serial.println("LED turned OFF");
}

// Toggle blinkmode
void handleBlink() {
  server.send(200, "text/html", "<h1>LED Blink Switch</h1>");
  Serial.println("LED Blink toogled");
  blinkLed = !blinkLed;
}

// handlermethod to toggle led
void handleToggle() {
  blinkLed = false;
  toggleLed();
}

// Toggle LED on or off
void toggleLed() {
    if(ledON) {
    turnLedOff();
  } else {
    turnLedOn();
  }
}

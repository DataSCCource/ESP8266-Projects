/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

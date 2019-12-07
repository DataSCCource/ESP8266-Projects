
#include <Servo.h>
#include <SPI.h>
#include "RF24.h"

Servo myservo;   // Servo-Objekt für den Servomotor
int pos = 0;    // Aktuelle Servo-Position

RF24 radio(8,7); // Radio-Objekt für das nRF24 Funkmodul
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  myservo.attach(3);  // attaches the servo on pin 3 to the servo object

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if(radio.available()) {
    radio.read(&pos, sizeof(pos));

    //Serial.print(pos);
    //Serial.print("\n");
    myservo.write(pos);
  } 
}

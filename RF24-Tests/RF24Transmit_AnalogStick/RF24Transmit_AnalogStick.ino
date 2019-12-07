
#include <SPI.h>
#include "RF24.h"

const int X_pin = 0; // analog pin connected to X output
int pos = 0;        // variable to store the joystik position

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(8,7);
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  Serial.print("Start...\n");

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  pos = map(analogRead(X_pin), 0,1023, 0,180);
  //Serial.print(pos);
  //Serial.print("\n");
  
  radio.write(&pos, sizeof(pos));
}

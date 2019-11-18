#include <FastLED.h>
#define LED_PIN     14
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  
}

int currentLed = 0;
int targetLed = 0;
long newLedCount = 0;
long newFlicker = 0;

void loop() {
  int r = 255;
  int g = 96;
  int b = 12;
  if(targetLed==currentLed) {
    targetLed = random(0,NUM_LEDS/2);
  }

  if(newLedCount < millis()) {
    newLedCount = millis() + random(10,25);
    if(currentLed < targetLed) {
      currentLed++;
    } else {
      currentLed--;
    }
  }
  
  if(newFlicker < millis()) {
    newFlicker = millis() + random(50,150);
    for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB (0,0,0);
    }
    
    for (int i = 0; i < NUM_LEDS-currentLed; i++) {
      int flicker = random(0,40);
      int r1 = r-flicker;
      int g1 = g-flicker;
      int b1 = b-flicker;
      if(b1<0) b1=0;
      
      leds[i] = CRGB (r1,g1,b1);
    }
    FastLED.show();
  }
}

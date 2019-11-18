### Wifi_MQTT-Client_dualLED-control
Wifi- and MQTT client to control brightness of two LEDs (warm and cold white)

Requires an wifi access point and MQTT broker (+ wiring for the LEDs)
__Requires library :__
* Board: https://github.com/esp8266/Arduino
* MQTT "PubSubClient": https://pubsubclient.knolleary.net/

### MQTT topics endpoints:
* __control/ledpanel/status__  
   Request current LED values. Answer topics will be:
   > * control/ledpanel/brightness
   > * control/ledpanel/colortemperature
* __control/ledpanel/fadebrightness__  
   Value 0-100:  Request to fade brightness in %
* __control/ledpanel/fadect__  
   Value 0-100:  Request to fade color temperature in %  
   0% = warm white | 50% = neutral white | 100% = cold white
* __control/ledpanel/fade__  
   Request fading to given brightness and color temperature  
   (e.g. 50+100 for 50% brightness and 100% = cold white)

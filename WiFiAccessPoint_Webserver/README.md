# WiFiAccessPoint_Webserver
ESP8266 Program that introduces a WiFi Access Point and a webserver.

Connected to the AP ("ESP8266-ap") you need to open http://192.168.4.1/ in a browser.  
__Requires board library :__ https://github.com/esp8266/Arduino

### Webserver endpoints:
* __/__  
   Main site with toggle and blink button
* __/an__  
   Turns LED on
* __/aus__  
   Turns LED off
* __/toogle__  
   Toogles LED state
* __/blink__  
   Activates blinkin mode with 500ms intervals

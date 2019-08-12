// LED will blink when in config mode

#include <WiFiManager.h> // NOTE: USED DEVELOPMENT BRANCH WHICH SUPPORTS ESP32  https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>

//for LED status
#include <Ticker.h>
Ticker ticker;

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15
#define PIXEL_STATUS_LED 0

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

uint32_t color_blue = pixels.Color(0, 0, 255);
uint32_t color_white = pixels.Color(255, 255, 255);
uint32_t color_off = pixels.Color(0, 0, 0);


void tick()
{
  //toggle state
  // set pin to the opposite state
  if ( pixels.getPixelColor(PIXEL_STATUS_LED) == color_off){
    pixels.setPixelColor(PIXEL_STATUS_LED, color_blue);
  } else {
    pixels.setPixelColor(PIXEL_STATUS_LED, color_off);
  }
  pixels.show();
  
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //set led pin as output
  pixels.begin(); 

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  // wm.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...");
  ticker.detach();
  //keep LED on
  pixels.setPixelColor(PIXEL_STATUS_LED, color_white);
  pixels.show();
}

void loop() {
  // put your main code here, to run repeatedly:

}
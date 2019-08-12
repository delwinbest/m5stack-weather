// LED will blink when in config mode

#include <WiFiManager.h> // NOTE: USED DEVELOPMENT BRANCH WHICH SUPPORTS ESP32  https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//for LED status
#include <Ticker.h>
Ticker ticker;

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15
#define PIXEL_STATUS_LED 0
#define TFT_BL          32  // goes to TFT BL
#define BLK_PWM_CHANNEL 7   // LEDC_CHANNEL_7
#define TFT_CS          14  // goes to TFT CS
#define TFT_DC          27  // goes to TFT DC
#define TFT_MOSI        23  // goes to TFT MOSI
#define TFT_SCLK        18  // goes to TFT SCK/CLK
#define TFT_RST         33  // goes to TFT RESET
#define TFT_MISO            // Not connected


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
// If using software SPI change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

uint32_t color_blue = pixels.Color(0, 0, 100);
uint32_t color_white = pixels.Color(100, 100, 100);
uint32_t color_off = pixels.Color(0, 0, 0);

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(wait);
  }
}

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
  // Init the back-light LED PWM
  ledcSetup(BLK_PWM_CHANNEL, 44100, 8);
  ledcAttachPin(TFT_BL, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 100);  // Set Brightness
  tft.begin();tft.setRotation(0);tft.fillScreen(ILI9341_BLACK); tft.setTextSize(2);tft.println("Starting...");delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  //set led pin as output
  pixels.begin(); 
  colorWipe(color_off, 50);
  
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

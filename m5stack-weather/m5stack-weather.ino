#include "settings.h"
#include <WiFiManager.h> // NOTE: USED DEVELOPMENT BRANCH WHICH SUPPORTS ESP32  https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>
#include "SPI.h"
#include "ILI9341_SPI.h"
#include "SunMoonCalc.h"

#include <JsonListener.h>
#include <OpenWeatherMapCurrent.h>
#include <OpenWeatherMapForecast.h>
#include <Astronomy.h>
#include "MiniGrafx.h"
#include "Carousel.h"

#include "ArialRounded.h"
#include "moonphases.h"
#include "weathericons.h"

#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_YELLOW 2
#define MINI_BLUE 3

#define MAX_FORECASTS 12

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C
                      }; //3

int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
// Limited to 4 colors due to memory constraints
int BITS_PER_PIXEL = 2; // 2^2 =  4 colors


//for LED status
#include <Ticker.h>
Ticker ticker;




Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
// If using software SPI change pins as desired
// TODO: Remove old adafruit lib

ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC, TFT_RST);
// ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette, 320, 240);
Carousel carousel(&gfx, 0, 0, 240, 100);

uint32_t color_blue = pixels.Color(0, 0, 100);
uint32_t color_white = pixels.Color(100, 100, 100);
uint32_t color_off = pixels.Color(0, 0, 0);

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
simpleDSTadjust dstAdjusted(StartRule, EndRule);
Astronomy::MoonData moonData;
//SunMoonCalc::Moon moonData;

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawWifiQuality();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawCurrentWeatherDetail();
void drawLabelValue(uint8_t line, String label, String value);
void drawForecastTable(uint8_t start);
void drawAbout();
void drawSeparator(uint16_t y);
String getTime(time_t *timestamp);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
void drawForecast3(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
FrameCallback frames[] = { drawForecast1, drawForecast2, drawForecast3 };
int frameCount = 3;

// how many different screens do we have?
int screenCount = 5;
long lastDownloadUpdate = millis();

uint16_t screen = 0;
long timerPress;
bool canBtnPress;
time_t dstOffset = 0;

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
  gfx.init();
  tft.setRotation(1);
  gfx.fillBuffer(MINI_BLACK);
  gfx.commit();
  
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

  Serial.println("Mounting file system...");
  bool isFSMounted = SPIFFS.begin();
  if (!isFSMounted) {
    Serial.println("Formatting file system...");
    drawProgress(50,"Formatting file system");
    SPIFFS.format();
    Serial.println("Formatting done");
  } else {
    Serial.println("File system mounted...");
  }
  drawProgress(100,"Formatting done");
  
  ticker.detach();
  //keep LED on
  pixels.setPixelColor(PIXEL_STATUS_LED, color_off);
  pixels.show();
}

void loop() {
  // put your main code here, to run repeatedly:

}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(20, 5, ThingPulseLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 90, "https://thingpulse.com");
  gfx.setColor(MINI_YELLOW);

  gfx.drawString(120, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 168, 240 - 20, 15);
  gfx.setColor(MINI_BLUE);
  gfx.fillRect(12, 170, 216 * percentage / 100, 11);

  gfx.commit();
}

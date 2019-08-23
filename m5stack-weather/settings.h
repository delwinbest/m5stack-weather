#include <simpleDSTadjust.h>
#include "secrets.h"
// CREAT SECRETS HEADER FILE WITH FOLLOW IN:
// OpenWeatherMap Settings
// Sign up here to get an API key: https://docs.thingpulse.com/how-tos/openweathermap-key/
// String OPEN_WEATHER_MAP_APP_ID = "XXXXXXXXXXXXXX"; // < insert openweather key


#define TFT_BL          32  // goes to TFT BL
#define BLK_PWM_CHANNEL 7   // LEDC_CHANNEL_7
#define TFT_CS          14  // goes to TFT CS
#define TFT_DC          27  // goes to TFT DC
#define TFT_MOSI        23  // goes to TFT MOSI
#define TFT_SCLK        18  // goes to TFT SCK/CLK
#define TFT_RST         33  // goes to TFT RESET
#define TFT_MISO        0    // Not connected
#define SERIAL2_RX_PIN      16
#define SERIAL2_TX_PIN      17
#define SERIAL2_RESET_PIN   5  

#define WIFI_HOSTNAME   "weather-station"
const int UPDATE_INTERVAL_SECS  = 10 * 60; // Update every 10 minutes
const int SLEEP_INTERVAL_SECS   = 0;        // Going to sleep after idle times, set 0 for insomnia

/*
Go to https://openweathermap.org/find?q= and search for a location. Go through the
result set and select the entry closest to the actual location you want to display 
data for. It'll be a URL like https://openweathermap.org/city/2657896. The number
at the end is what you assign to the constant below.
 */
String OPEN_WEATHER_MAP_LOCATION_ID = "1852140";
String DISPLAYED_CITY_NAME = "Shinagawa";
/*
Arabic -> ar, Bulgarian -> bg, Catalan -> ca, Czech -> cz, German -> de, Greek -> el,
English -> en, Persian (Farsi) -> fa, Finnish -> fi, French -> fr, Galician -> gl,
Croatian -> hr, Hungarian -> hu, Italian -> it, Japanese -> ja, Korean -> kr,
Latvian -> la, Lithuanian -> lt, Macedonian -> mk, Dutch -> nl, Polish -> pl,
Portuguese -> pt, Romanian -> ro, Russian -> ru, Swedish -> se, Slovak -> sk,
Slovenian -> sl, Spanish -> es, Turkish -> tr, Ukrainian -> ua, Vietnamese -> vi,
Chinese Simplified -> zh_cn, Chinese Traditional -> zh_tw.
*/
String OPEN_WEATHER_MAP_LANGUAGE = "en";
const uint8_t MAX_FORECASTS = 10;

// Adjust according to your language
const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
const String SUN_MOON_TEXT[] = {"Sun", "Rise", "Set", "Moon", "Age", "Illum"};
const String MOON_PHASES[] = {"New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
                              "Full Moon", "Waning Gibbous", "Third quarter", "Waning Crescent"};

#define UTC_OFFSET +9
struct dstRule StartRule = {"JST", Last, Sun, Mar, 2, 0}; // no dst in JP
struct dstRule EndRule = {"JST", Last, Sun, Oct, 2, 0};      

// values in metric or imperial system?
const boolean IS_METRIC = true;

// Change for 12 Hour/ 24 hour style clock
bool IS_STYLE_12HR = false;

// change for different NTP (time servers)
#define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"
// #define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"

// August 1st, 2018
#define NTP_MIN_VALID_EPOCH 1533081600

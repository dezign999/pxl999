//Derived from peerdavid's source at: https://github.com/peerdavid/Watchy
#ifndef WATCHY_BASE_H
#define WATCHY_BASE_H

#include <Watchy.h>

#if __has_include("config.h") && __has_include(<stdint.h>)
# include "config.h"
#endif

extern RTC_DATA_ATTR bool twelve_mode;
extern RTC_DATA_ATTR bool runOnce;
extern RTC_DATA_ATTR bool delayedStart;
extern RTC_DATA_ATTR bool firstNTP;
extern RTC_DATA_ATTR bool debugger;

//weather api - Update these to match your city/country/api key
//get your free api key from: https://openweathermap.org/appid
//#define CITY_ID "5378538" // if your city name isn't very unique, you can use its ID instead
#define CITY "WADING+RIVER" //if your city name has a space, replace with '+'
#define COUNTRY "US"
#define APIKEY "f058fe1cad2afe8e2ddc5d063a64cecb" //use your own API key (this is SQFMI'S) :)
#define URL "http://api.openweathermap.org/data/2.5/weather?q="
#define TEMP "imperial" //use "imperial" for Fahrenheit or "metric" for Celcius

//NTP Syncing - updates your time twice a day to fix any drifting
//Get your timezone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "nist.time.gov"
#define NTP_SERVER_3 "0.ie.pool.ntp.org"

// Btn definitions
#define IS_BTN_LEFT_UP      (wakeupBit & BACK_BTN_MASK && guiState == WATCHFACE_STATE)
#define EXT_INT_MASK        BACK_BTN_MASK

class WatchyBase : public Watchy {
    public:
        WatchyBase();
        virtual void init();
        virtual void handleButtonPress();
        virtual void deepSleep();
        void vibrate(uint8_t times=1, uint32_t delay_time=50);
        esp_sleep_wakeup_cause_t wakeup_reason;
        weatherData getWeather();
        void syncNtpTime();
    private:
        void _rtcConfig();
        void _bmaConfig();
        static uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
        static uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
};

#endif

#include "Watchy_pxl999.h"

#define TIME_FONT timeLGMono42pt7b
#define DATE_FONT timeLGMono20pt7b
#define SMALL_TEXT smTextMono8pt7b

RTC_DATA_ATTR int16_t weatherConditionCode;
RTC_DATA_ATTR int8_t temperature;
RTC_DATA_ATTR bool showCached = false;

//for night weather icons ;)
bool isNight = false;

//time between weather syncs
#define WEATHER_TIMER 30 

WatchyPXL999::WatchyPXL999(){}

void WatchyPXL999::drawTime(){
  
  String hourStr = String(currentTime.Hour);
  String minStr = String(currentTime.Minute);
  
  hourStr = (twelve_mode && currentTime.Hour > 12 &&  currentTime.Hour <= 21) ? "0" + String(currentTime.Hour - 12) : (twelve_mode && currentTime.Hour > 12) ? String(currentTime.Hour - 12) : 
            (twelve_mode && currentTime.Hour == 0) ? "12" : currentTime.Hour < 10 ? "0" + hourStr : hourStr;
  minStr = currentTime.Minute < 10 ? "0" + minStr : minStr;

  display.setFont(&TIME_FONT);
  display.setTextColor(GxEPD_WHITE);

  //Hour
  display.setCursor(16, 87);
  display.print(hourStr);

  //Minute    
  display.setCursor(16, 148);
  display.print(minStr);
}

void WatchyPXL999::drawDate(){
  String dayName = dayStr(currentTime.Wday);
  String monthName = monthStr(currentTime.Month);
  String dateStr = String(currentTime.Day);
  dateStr = currentTime.Day < 10 ? "0" + dateStr : dateStr;

  display.setFont(&DATE_FONT);
  display.setCursor(16, 184);
  display.print(dateStr);

  display.setFont(&SMALL_TEXT);
  display.setCursor(76, 169);
  display.print(monthName);

  display.setCursor(76, 184);
  display.print(dayName);
}

void WatchyPXL999::drawWeatherIcon() {
    
    const unsigned char* weatherIcon;

    //https://openweathermap.org/weather-conditions
    if(weatherConditionCode == 999){//RTC
      weatherIcon = rtc;
    }else if(weatherConditionCode > 801 &&weatherConditionCode < 805){//Cloudy
      weatherIcon = scatteredclouds;
    }else if(weatherConditionCode == 801){//Few Clouds
      weatherIcon = (isNight) ? fewcloudsnight : fewclouds;
    }else if(weatherConditionCode == 800){//Clear
      weatherIcon = (isNight) ? clearskynight : clearsky;
    }else if(weatherConditionCode >=700){//Atmosphere
      weatherIcon = mist; 
    }else if(weatherConditionCode >=600){//Snow
      weatherIcon = snow;
    }else if(weatherConditionCode >=500){//Rain
      weatherIcon = rain;  
    }else if(weatherConditionCode >=300){//Drizzle
      weatherIcon = drizzle;
    }else if(weatherConditionCode >=200){//Thunderstorm
      weatherIcon = thunderstorm; 
    }
    display.drawBitmap(143, 93, weatherIcon, 45, 40, GxEPD_WHITE);
}

void WatchyPXL999::drawWeather(){
    
    if(showCached == false) {
        weatherData latestWeather = getWeather();
        temperature = latestWeather.temperature;
        weatherConditionCode = latestWeather.weatherConditionCode;
    } else {
      showCached = false;
    }

    display.setFont(&SMALL_TEXT);
    //Get width of text & center it under the weather icon. 166 is the centerpoint of the icon
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(temperature) + ".", 45, 13, &x1, &y1, &w, &h);
    display.setCursor(166 - w/2, 148);
    if(debugger)
        Serial.println(String(temperature));
    display.println(String(temperature) + ".");
}

void WatchyPXL999::drawWatchFace(){
    
   display.fillScreen(GxEPD_BLACK);
   
   drawTime();
   drawDate();

   isNight = (currentTime.Hour >= 19 || currentTime.Hour <= 5) ? true : false;
   
   if(delayedStart || currentTime.Minute % WEATHER_TIMER == 0) {
      if(debugger) {
            Serial.println("getting new weather");
            Serial.println("initial runOnce: " + String(runOnce));
      }
      if(runOnce) {
        runOnce = false;
        delayedStart = false;
      }
      drawWeather();
   } else if (!runOnce) {
      if(debugger)
          Serial.println("showing cached weather");
      showCached = true;
      drawWeather();
   }
   if(runOnce) {
    //this is a SILLY workaround to prevent the watchy from getting stuck in a loop
    //from running connectWiFI too early. I mean, is it silly if it works? XD
      if(debugger)
          Serial.println("getting RTC weather");
      
      drawWeather();
      delay(20000);
      delayedStart = true;
   }

   drawWeatherIcon();
   
   if(debugger) {
      Serial.println("last runOnce: " + String(runOnce));
      Serial.println("delayedStart: " + String(delayedStart));
   }

    //another silly work around for reducing ghosting
   for(int i = 0; i < 3; i++){
    display.display(true);
   }

}

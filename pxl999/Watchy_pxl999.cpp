#include "Watchy_pxl999.h"

#define TIME_FONT timeLGMono42pt7b
#define DATE_FONT timeLGMono20pt7b
#define SMALL_TEXT smTextMono8pt7b


RTC_DATA_ATTR bool showCached = false;
RTC_DATA_ATTR bool pauseEnabled = false;
RTC_DATA_ATTR bool delayedStart = false;
RTC_DATA_ATTR bool runOnce = true;

//Pause weather updates when the watch is not in use to conserve battery life.
//Time is defined in 24h format. Set both times to "0:00" to disable pausing.
String pauseStart = "0:30"; //Stops weather updates at 12:30am
String pauseEnd = "5:45"; //Resumes weather updates at 5:45am

//Night weather icons ;)
bool isNight = false;

//Time between weather syncs in minutes
#define WEATHER_TIMER 30

//Time between NTP syncs in hours
#define NTP_TIMER 12

WatchyPXL999::WatchyPXL999() {}

void WatchyPXL999::drawTime() {

  String hourStr = String(currentTime.Hour);
  String minStr = String(currentTime.Minute);

  hourStr = (twelve_mode && currentTime.Hour > 12 &&  currentTime.Hour <= 21) ? "0" + String(currentTime.Hour - 12) : (twelve_mode && currentTime.Hour > 12) ? String(currentTime.Hour - 12) :
            (twelve_mode && currentTime.Hour == 0) ? "12" : currentTime.Hour < 10 ? "0" + hourStr : hourStr;
  minStr = currentTime.Minute < 10 ? "0" + minStr : minStr;

  display.setFont(&TIME_FONT);
  display.setTextColor(GxEPD_WHITE);

  display.fillRect(11, 27, 128, 124, GxEPD_BLACK); //Redraw Helper

  //Hour
  display.setCursor(16, 87);
  display.print(hourStr);

  //Minute
  display.setCursor(16, 148);
  display.print(minStr);
}

void WatchyPXL999::drawDate() {
  String dayName = dayStr(currentTime.Wday);
  String monthName = monthStr(currentTime.Month);
  String dateStr = String(currentTime.Day);
  dateStr = currentTime.Day < 10 ? "0" + dateStr : dateStr;

  display.fillRect(11, 153, 178, 38, GxEPD_BLACK); //Redraw Helper

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

  isNight = (currentTime.Hour >= 18 || currentTime.Hour <= 5) ? true : false;
  const unsigned char* weatherIcon;

  //https://openweathermap.org/weather-conditions
  if (weatherConditionCode == 999) { //RTC
    weatherIcon = rtc;
  } else if (weatherConditionCode == 998) { //RTC SLEEEP
    weatherIcon = rtcsleep;
  } else if (weatherConditionCode > 801 && weatherConditionCode < 805) { //Cloudy
    weatherIcon = scatteredclouds;
  } else if (weatherConditionCode == 801) { //Few Clouds
    weatherIcon = (isNight) ? fewcloudsnight : fewclouds;
  } else if (weatherConditionCode == 800) { //Clear
    weatherIcon = (isNight) ? clearskynight : clearsky;
  } else if (weatherConditionCode >= 700) { //Atmosphere
    weatherIcon = mist;
  } else if (weatherConditionCode >= 600) { //Snow
    weatherIcon = snow;
  } else if (weatherConditionCode >= 500) { //Rain
    weatherIcon = rain;
  } else if (weatherConditionCode >= 300) { //Drizzle
    weatherIcon = drizzle;
  } else if (weatherConditionCode >= 200) { //Thunderstorm
    weatherIcon = thunderstorm;
  }

  display.fillRect(141, 91, 49, 44, GxEPD_BLACK); //Redraw Helper
  display.drawBitmap(143, 93, weatherIcon, 45, 40, GxEPD_WHITE);
}

void WatchyPXL999::drawWeather() {
  if (!pauseUpdates() && !delayedStart) {
    if (debugger)
      Serial.println("Resuming Updates");
    if (showCached == false) {
      weatherData latestWeather = getWeather();
      temperature = latestWeather.temperature;
      weatherConditionCode = latestWeather.weatherConditionCode;
    } else {
      showCached = false;
    }
  } else {
    temperature = rtcTemp();
    latestWeather.temperature = temperature;
    weatherConditionCode = (pauseUpdates()) ? 998 : 999;
    latestWeather.weatherConditionCode = weatherConditionCode;
    cityNameID = 999;
    if (debugger) {
      Serial.println("Paused Updates");
      Serial.println("RTC Temp - latestWeather.temp: " + String(latestWeather.temperature));
    }
  }

  display.setFont(&SMALL_TEXT);
  display.fillRect(142, 136, 49, 13, GxEPD_BLACK); //Redraw Helper
  //Get width of text & center it under the weather icon. 165 is the centerpoint of the icon
  int16_t  x1, y1;
  uint16_t w, h;
  display.getTextBounds(String(temperature) + ".", 45, 13, &x1, &y1, &w, &h);
  display.setCursor(166 - w / 2, 148);
  if (debugger)
    Serial.println("Latest temperature: " + String(temperature));
  display.println(String(temperature) + ".");

  cityName = getCityAbbv();
  display.fillRect(142, 77, 49, 13, GxEPD_BLACK); //Redraw Helper
  display.getTextBounds(cityName, 45, 13, &x1, &y1, &w, &h);
  display.setCursor(165 - w / 2, 87);
  if (debugger)
    Serial.println("Current City : " + String(cityName) + " | " + getCityName());
  display.println(cityName);

  if (debugger) { //show active weather condition code
    String weathercode = String(weatherConditionCode);
    display.getTextBounds(weathercode, 45, 13, &x1, &y1, &w, &h);
    display.setCursor(165 - w / 2, 55);
    display.println(weathercode);
  }

}

bool WatchyPXL999::pauseUpdates() {

  //Get Times from String
  int sColon = pauseStart.indexOf(':');
  int eColon = pauseEnd.indexOf(':');

  int pauseHour = (pauseStart.substring(0, sColon)).toInt();
  int pauseMin = (pauseStart.substring(sColon + 1)).toInt();

  int endHour = (pauseEnd.substring(0, eColon)).toInt();
  int endMin = (pauseEnd.substring(eColon + 1)).toInt();

  if (currentTime.Hour == pauseHour && currentTime.Minute == pauseMin) {
    pauseEnabled = true;
    if (debugger)
      Serial.println("Enabling pauseEnabled: " + String(pauseEnabled));
  }

  if (currentTime.Hour == endHour && currentTime.Minute == endMin) {
    pauseEnabled = false;
    runOnce = true;
    if (debugger)
      Serial.println("Disabling pauseEnabled: " + String(pauseEnabled));
  }

  if (debugger) {
    Serial.println("startHour: " + String(pauseHour));
    Serial.println("startMin: " + String(pauseMin));
    Serial.println("endHour: " + String(endHour));
    Serial.println("endMin: " + String(endMin));
    Serial.println("pauseEnabled: " + String(pauseEnabled));
  }
  return pauseEnabled;
}

void WatchyPXL999::drawWatchFace() {

  display.fillScreen(GxEPD_BLACK);
  drawTime();
  drawDate();

  if (!pauseUpdates()) { //Check if live updates aren't paused

    if (delayedStart) { //Sync Weather & NTP on second Tick to avoid crashing Watchy on first launch
      delayedStart = false;
      drawWeather();
      syncNtpTime();
      if (debugger) {
        Serial.println("Delayed Start. Syncing Weather & NTP");
        Serial.println("initial runOnce: " + String(runOnce));
        Serial.println("Initial NTP Sync");
      }
    }

    if (currentTime.Minute % WEATHER_TIMER == 0 || currentTime.Hour % NTP_TIMER == 0 && currentTime.Minute == 0) { //Check time to sync Weather or NTP

      if (currentTime.Minute % WEATHER_TIMER == 0) { //Sync Weather
        if (debugger)
          Serial.println("getting new weather");
        drawWeather();
        //syncNtpTime();
      }

      if (currentTime.Hour % NTP_TIMER == 0 && currentTime.Minute == 0) { //Sync NTP
        if (debugger)
          Serial.println("Getting new NTP time");
        syncNtpTime();
      }

    } else { //Not time to sync, show cached weather
      if (debugger)
        Serial.println("showing cached weather");
      showCached = true;
      drawWeather();
    }

    if (runOnce) {
      //this is a SILLY workaround to prevent the watchy from getting stuck in a loop
      //when checking the weather too quickly. I mean, is it silly if it works? XD
      if (debugger)
        Serial.println("getting RTC weather first");
      delayedStart = true; //Sync on next tick
      runOnce = false;
      drawWeather();
    }

  } else { //Live updates disabled, show RTC temp and icon
    if (debugger)
      Serial.println("Weather Paused, getting RTC Temp");
    drawWeather();
  }

  drawWeatherIcon();
  disableWiFi();

  //another silly work around to help reduce ghosting
  for (int i = 0; i < 3; i++) {
    display.display(true);
  }

}

#include "Watchy_pxl999.h"

#define TIME_FONT timeLGMono42pt7b
#define DATE_FONT timeLGMono20pt7b
#define SMALL_TEXT smTextMono8pt7b

RTC_DATA_ATTR int16_t weatherConditionCode;
RTC_DATA_ATTR int8_t temperature;
RTC_DATA_ATTR bool showCached = false;
RTC_DATA_ATTR bool pauseEnabled = false;

//Pause weather updates when the watch is not in use to conserve battery life.
//Time is defined in 24h format. Set both times to "0:00" to disable pausing.
String pauseStart = "0:30"; //Stops weather updates at 12:30am
String pauseEnd = "5:30"; //Resumes weather updates at 5:30am

//for night weather icons ;)
bool isNight = false;

//time between weather syncs
#define WEATHER_TIMER 30

WatchyPXL999::WatchyPXL999() {}

void WatchyPXL999::drawTime() {

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

void WatchyPXL999::drawDate() {
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
  display.drawBitmap(143, 93, weatherIcon, 45, 40, GxEPD_WHITE);
}

void WatchyPXL999::drawWeather() {
  if (!pauseUpdates()) {
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
    if (debugger)
      Serial.println("Paused Updates");
    temperature = RTC.temperature() / 4; //celsius
    if (strcmp(TEMP, "imperial") == 0) {
      temperature = temperature * 9. / 5. + 32.; //fahrenheit
    }
    if (debugger)
      Serial.println("RTC Temp: " + String(temperature));
    latestWeather.temperature = temperature;
    weatherConditionCode = 998;
  }

  display.setFont(&SMALL_TEXT);
  //Get width of text & center it under the weather icon. 166 is the centerpoint of the icon
  int16_t  x1, y1;
  uint16_t w, h;
  display.getTextBounds(String(temperature) + ".", 45, 13, &x1, &y1, &w, &h);
  display.setCursor(166 - w / 2, 148);
  if (debugger)
    Serial.println("Current temperature: " + String(temperature));
  display.println(String(temperature) + ".");
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

  isNight = (currentTime.Hour >= 19 || currentTime.Hour <= 5) ? true : false;
  if (!pauseUpdates()) {
    if (delayedStart || currentTime.Minute % WEATHER_TIMER == 0) {
      if (debugger) {
        Serial.println("getting new weather");
        Serial.println("initial runOnce: " + String(runOnce));
      }
      if (runOnce) {
        runOnce = false;
        delayedStart = false;
      }
      drawWeather();
    } else if (!runOnce && !firstNTP) {
      if (debugger)
        Serial.println("showing cached weather");
      showCached = true;
      drawWeather();
    }
    if (runOnce) {
      //this is a SILLY workaround to prevent the watchy from getting stuck in a loop
      //from running connectWiFI too early. I mean, is it silly if it works? XD
      if (debugger) {
        Serial.println("getting RTC weather");
        Serial.println("2 Second Delay :(");
      }

      drawWeather();
      delay(20000);
      delayedStart = true;
    }
  } else {
    //Weather Updates Paused
    if (debugger)
      Serial.println("Weather Paused, getting RTC Temp");
    drawWeather();
  }

  drawWeatherIcon();

  if (debugger) {
    Serial.println("last runOnce: " + String(runOnce));
    Serial.println("delayedStart: " + String(delayedStart));
  }

  //another silly work around for reducing ghosting
  for (int i = 0; i < 3; i++) {
    display.display(true);
  }

}

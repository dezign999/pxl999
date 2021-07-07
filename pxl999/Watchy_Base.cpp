//Derived from peerdavid's source at: https://github.com/peerdavid/Watchy
#include "Watchy_Base.h"

RTC_DATA_ATTR bool twelve_mode = true;
RTC_DATA_ATTR bool sleep_mode = false;
RTC_DATA_ATTR bool runOnce = true;
RTC_DATA_ATTR bool delayedStart = false;
RTC_DATA_ATTR bool firstNTP = true;

//Set this flag to true if you want to monitor Serial logs
RTC_DATA_ATTR bool debugger = false;

RTC_DATA_ATTR weatherData latestWeather;

WatchyBase::WatchyBase() {}

void WatchyBase::init() {
  
  if(debugger)
      Serial.begin(115200);
  
  wakeup_reason = esp_sleep_get_wakeup_cause(); //get wake up reason
  Wire.begin(SDA, SCL); //init i2c

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm

      // Handle classical tick
      RTC.alarm(ALARM_2); //resets the alarm flag in the RTC

      if (guiState == WATCHFACE_STATE) {
        RTC.read(currentTime);
        showWatchFace(true); //partial updates on tick
      }
      break;

    case ESP_SLEEP_WAKEUP_EXT1: //button Press + no handling if wakeup
      if (sleep_mode) {
        sleep_mode = false;
        RTC.alarmInterrupt(ALARM_2, true);
        RTC.alarm(ALARM_2); //resets the alarm flag in the RTC

        RTC.read(currentTime);
        showWatchFace(false); //full update on wakeup from sleep mode
        break;
      }

      handleButtonPress();
      break;

    default: //reset
      _rtcConfig();
      _bmaConfig();
      showWatchFace(true); //full update on reset
      break;
  }

  // Sometimes BMA crashes - simply try to reinitialize bma...
  if (sensor.getErrorCode() != 0) {
    sensor.shutDown();
    sensor.wakeUp();
    sensor.softReset();
    _bmaConfig();
  }
  deepSleep();
}

void WatchyBase::deepSleep() {
  esp_sleep_enable_ext0_wakeup(RTC_PIN, 0); //enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(EXT_INT_MASK, ESP_EXT1_WAKEUP_ANY_HIGH); //enable deep sleep wake on button press
  esp_deep_sleep_start();
}

void WatchyBase::handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

  if (IS_BTN_LEFT_UP) {
    twelve_mode = (twelve_mode == 0) ? true : false;
    RTC.read(currentTime);
    vibrate();
    showWatchFace(true);
    return;
  }

  Watchy::handleButtonPress();

}

void WatchyBase::vibrate(uint8_t times, uint32_t delay_time) {
  // Ensure that no false positive double tap is produced
  sensor.enableFeature(BMA423_WAKEUP, false);

  pinMode(VIB_MOTOR_PIN, OUTPUT);
  for (uint8_t i = 0; i < times; i++) {
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, true);
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, false);
  }

  sensor.enableFeature(BMA423_WAKEUP, true);
}

void WatchyBase::_rtcConfig() {
  //https://github.com/JChristensen/DS3232RTC
  RTC.squareWave(SQWAVE_NONE); //disable square wave output
  //RTC.set(compileTime()); //set RTC time to compile time
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0); //alarm wakes up Watchy every minute
  RTC.alarmInterrupt(ALARM_2, true); //enable alarm interrupt
  RTC.read(currentTime);
}

void WatchyBase::_bmaConfig() {

  if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
    //fail to init BMA
    return;
  }

  // Accel parameter structure
  Acfg cfg;
  /*!
      Output data rate in Hz, Optional parameters:
          - BMA4_OUTPUT_DATA_RATE_0_78HZ
          - BMA4_OUTPUT_DATA_RATE_1_56HZ
          - BMA4_OUTPUT_DATA_RATE_3_12HZ
          - BMA4_OUTPUT_DATA_RATE_6_25HZ
          - BMA4_OUTPUT_DATA_RATE_12_5HZ
          - BMA4_OUTPUT_DATA_RATE_25HZ
          - BMA4_OUTPUT_DATA_RATE_50HZ
          - BMA4_OUTPUT_DATA_RATE_100HZ
          - BMA4_OUTPUT_DATA_RATE_200HZ
          - BMA4_OUTPUT_DATA_RATE_400HZ
          - BMA4_OUTPUT_DATA_RATE_800HZ
          - BMA4_OUTPUT_DATA_RATE_1600HZ
  */
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  /*!
      G-range, Optional parameters:
          - BMA4_ACCEL_RANGE_2G
          - BMA4_ACCEL_RANGE_4G
          - BMA4_ACCEL_RANGE_8G
          - BMA4_ACCEL_RANGE_16G
  */
  cfg.range = BMA4_ACCEL_RANGE_2G;
  /*!
      Bandwidth parameter, determines filter configuration, Optional parameters:
          - BMA4_ACCEL_OSR4_AVG1
          - BMA4_ACCEL_OSR2_AVG2
          - BMA4_ACCEL_NORMAL_AVG4
          - BMA4_ACCEL_CIC_AVG8
          - BMA4_ACCEL_RES_AVG16
          - BMA4_ACCEL_RES_AVG32
          - BMA4_ACCEL_RES_AVG64
          - BMA4_ACCEL_RES_AVG128
  */
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /*! Filter performance mode , Optional parameters:
      - BMA4_CIC_AVG_MODE
      - BMA4_CONTINUOUS_MODE
  */
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;

  // Configure the BMA423 accelerometer
  sensor.setAccelConfig(cfg);

  // Enable BMA423 accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  sensor.enableAccel();

  struct bma4_int_pin_config config ;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl = BMA4_ACTIVE_HIGH;
  config.od = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en = BMA4_INPUT_DISABLE;
  // The correct trigger interrupt needs to be configured as needed
  sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis = 2;
  remap_data.z_axis_sign = 0xFF;
  // Need to raise the wrist function, need to set the correct axis
  sensor.setRemapAxes(&remap_data);

  // Enable BMA423 isStepCounter feature
  sensor.enableFeature(BMA423_STEP_CNTR, true);
  // Enable BMA423 isTilt feature
  sensor.enableFeature(BMA423_TILT, true);
  // Enable BMA423 isDoubleClick feature
  //sensor.enableFeature(BMA423_WAKEUP, true);

  // Reset steps
  //sensor.resetStepCounter();

  // Turn on feature interrupt
  //sensor.enableStepCountInterrupt();
  //sensor.enableTiltInterrupt();
  // It corresponds to isDoubleClick interrupt
  //sensor.enableWakeupInterrupt();
}
uint16_t WatchyBase::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t WatchyBase::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 !=  Wire.endTransmission());
}

weatherData WatchyBase::getWeather(){

        if(!runOnce && connectWiFi()){//Use Weather API for live data if WiFi is connected
            HTTPClient http;
            http.setConnectTimeout(3000);//3 second max timeout
        #ifdef CITY_ID
            String weatherQueryURL = String(URL) + String("?id=")+ String(CITY_ID) + String("&units=") + String(TEMP) + String("&appid=") + String(APIKEY);
        #else            
            String weatherQueryURL = String(URL) + String("?q=") + String(CITY) + String(",") + String(COUNTRY) + String("&units=") + String(TEMP) + String("&appid=") + String(APIKEY);
        #endif            
            http.begin(weatherQueryURL.c_str());
            int httpResponseCode = http.GET();
            if(httpResponseCode == 200) {
                String payload = http.getString();
                JSONVar responseObject = JSON.parse(payload);
                latestWeather.temperature = int(responseObject["main"]["temp"]);
                latestWeather.weatherConditionCode = int(responseObject["weather"][0]["id"]);

            if(firstNTP || currentTime.Hour % 12 == 0 && currentTime.Minute == 0) {
              syncNtpTime();
              if(debugger) {
                  Serial.println("Initial NTP Sync");
                  Serial.println("firstNTP: " + String(firstNTP));
              }
              firstNTP = false;
            }
                
            }else{
                //http error
            }
            
            http.end();
            //turn off radios
            WiFi.mode(WIFI_OFF);
            btStop();
        }else{
            //No WiFi, use RTC Temperature
            uint8_t temperature = RTC.temperature() / 4; //celsius
            if(strcmp(TEMP, "imperial") == 0){
                temperature = temperature * 9. / 5. + 32.; //fahrenheit
            }
            latestWeather.temperature = temperature;
            latestWeather.weatherConditionCode = 999;
        } 
    return latestWeather;
}

//Derived from Symptym's snippet posted to the Watchy Discord
//https://gist.github.com/Symptym/0336f3f3d74dc66fe05e8d232bed3704
void WatchyBase::syncNtpTime() {
  
      if(debugger)
          Serial.println("Checking NTP time");
      configTzTime(TIMEZONE_STRING, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
      int i = 0;
      while (time(nullptr) < 1000000000l && i < 40) {
        delay(500);
        i++;
      }
      time_t tnow = time(nullptr);
      struct tm *local = localtime(&tnow);

      if(debugger){
          Serial.println("NTP Retrieved");
          Serial.print("Date: ");
          Serial.print(local->tm_year + 1900);
          Serial.print("-");
          if ((local->tm_mon + 1) < 10) {
            Serial.print("0");
          }
          Serial.print(local->tm_mon + 1);
          Serial.print("-");
          if (local->tm_mday < 10) {
            Serial.print("0");
          }
          Serial.println(local->tm_mday);
          Serial.print("Time: ");
          if (local->tm_hour < 10) {
            Serial.print("0");
          }
          Serial.print(local->tm_hour);
          Serial.print(":");
          if (local->tm_min < 10) {
            Serial.print("0");
          }
          Serial.print(local->tm_min);
          Serial.print(":");
          if (local->tm_sec < 10) {
            Serial.print("0");
          }
          Serial.println(local->tm_sec);
          Serial.print("Week Day: ");
          Serial.println(local->tm_wday);
      }

      currentTime.Year = local->tm_year + YEAR_OFFSET - 2000; //This change matches watchy defaults
      currentTime.Month = local->tm_mon + 1;
      currentTime.Day = local->tm_mday;
      currentTime.Hour = local->tm_hour;
      currentTime.Minute = local->tm_min;
      currentTime.Second = local->tm_sec;
      currentTime.Wday = local->tm_wday + 1;
      RTC.write(currentTime);
      RTC.read(currentTime);

}

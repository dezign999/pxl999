// Put your Access Point SSIDs and correpsonding passwords below. Put your most used accesspoint first.

const char* accessPoints[] = {"SSID-NAME-1", "SSID-NAME-2", "SSID-NAME-3"};
const char* apPasswords[] = {"SSID-PASS-1", "SSID-PASS-2", "SSID-PASS-3"};

// This is the abbreviated name that will appear above your weather icon to identify which access point you're connected to.
// Maximum of 4 letters.
const char* cityAbbv[] = {"HOME", "GPT" , "PHNE"};

// Place your city names below that correspond with the access point you,re connected to.
// This will show the weather for that access point you're connected to.
// If your city name isn't very unique, you can use its ID instead.
// When you look up a city on openweathermap.org, it'll take you to a page like
// "https://openweathermap.org/city/4574324". Copy those numbers (ex 4574324) on the end
// and put them between quotes where you'd normally put a cit name.
// If your city name has a space in it, use a + (plus sign) to join them, ex "WADING+RIVER".
// The plus sign is required so the web query url remains intact.

const char* cityNames[] = {"5143022", "GREENPORT" , "RIVERHEAD"};

const int accessPointCount = sizeof(accessPoints) / sizeof(accessPoints[0]); // number of known networks
RTC_DATA_ATTR int cityNameID;
RTC_DATA_ATTR String cityName;

String WatchyBase::getCityAbbv() {
  String scity;
  if (cityNameID == 999) {
    scity = "RTC";
  } else {
    scity = cityAbbv[cityNameID];
  }
  return scity;
}

String WatchyBase::getCityName() {
  String scity;
  if (cityNameID == 999) {
    scity = "RTC";
  } else {
    scity = cityNames[cityNameID];
  }
  return scity;
}

void WatchyBase::disableWiFi() {
  WiFi.mode(WIFI_OFF);
  WIFI_CONFIGURED = false;
  btStop();
  if (debugger)
    Serial.println("WiFi Turned Off. IP Check: " + WiFi.localIP());
}

bool WatchyBase::connectWiFi() {

  int i, n;

  for (n = 0; n < accessPointCount; n++) {
    Serial.println("Trying: " + String(accessPoints[n]));
    WiFi.begin(accessPoints[n], apPasswords[n]);
    i = 0;
    while (i < 10 && WiFi.status() != WL_CONNECTED) {
      i++;
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
      break;
    if (debugger) {
      Serial.print(F("\nConnecting to "));
      Serial.println(accessPoints[n]);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (debugger) {
      Serial.println(F("\nWiFi connected, your IP address is "));
      Serial.println(WiFi.localIP());
      Serial.println(cityNames[n]);
    }
    cityNameID = n;
    WIFI_CONFIGURED = true;
  } else {
    WIFI_CONFIGURED = false;
    disableWiFi();
  }

  if (debugger) {
    Serial.print("WIFI_CONFIGURED = ");
    Serial.println((WIFI_CONFIGURED == 1) ? "true" : "false");
  }
  return WIFI_CONFIGURED;
}

# pxl999
![alt text](https://github.com/dezign999/pxl999/blob/main/pxl999.gif?raw=true?raw=true)

Pxl999 is a chunky pixel watch face for SQFMI's Watchy. This watch face features live weather updates every 30 minutes and NTP syncing twice a day to account for any time drifting.

When internet access is not available, the RTC will be used for ambient temperature, and is indicated by a microchip as the weather icon.

This watch face was made using the free Arduino IDE, Photoshop CC, image2ccp, Glypher Studio (font creation), Cloud Convert (otf > ttf), and truetype2gfx.

I'm still relatively new to C++ (and rusty), so please excuse the mess. I've taken a lot of David Peer's code from his examples and borrowed much of Symptym's NTP time syncing code.

Please note, that this works on my watchy, your results may vary :P I had a real problem with the watchy crashing if I polled for weather too early at boot, therefore I put a workaround in by forcing a delay and just showing RTC temperature at first boot. If you find a way to fix this, please lmk!

## Configuration
Admittedly, there's quite a bit to configure with this (and future) watch face(s). These required configurations have comment descriptions in the source code. Below are the most critical settings that need to be configured in order to run this watch face:

### Weather
##### API KEY
In order for the watchy to receive weather upates, you'll need an API Key from [OpenWeatherMap.org](https://openweathermap.org/appid) which is completely free. I've included SQFMI's key which is enabled by default. Using this key is not recommended as it will cease to function when minute/daily limits are reached, or if SQFMI simply deletes it. Add your API Key to the `APIKEY` definition.

#### City Name or ID
Weather for your location is defined by entering your city name in the following array: `cityNames[] = {"5143022", "GREENPORT" , "RIVERHEAD"}`. If your city name is used in other states or provinces, you can also use your city code by looking up a city on openweathermap.org, it'll take you to a page like "https://openweathermap.org/city/4574324". Copy those numbers (ex 4574324) on the end and put them into the array between the quotes. Laslty, You'll also have to define `COUNTRY` and `TEMP`. `TEMP` is used to determine if Imperial temperature is displayed.

#### Pausing Weather Updates
It's possible to pause weather updates when the watch is not in use, i.e. when you're sleeping. By default, its set to pause 30 minutes after midnight, and resume updates at 5:30am. These can be changed by modifying the `pauseStart` and `pauseEnd` strings. Setting both strings to the same time will disable the pause updates feature. Pause start and end times must be written in the 24 hour format.

When updates are paused, the weather icon will be represented by a sleeping RTC icon and ambient temperature will be used instead of live weather.

## NTP
#### Timezone
NTP syncs require that you define your timezone, `TIMEZONE_STRING` which can be found [using this resource link](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json). Use the example in the source to determine what is required. 

#### NTP Servers
By default, the NTP server tried first is a pool of servers that should automatically find the best match based on your location. You can leave `NTP_SERVER_1` as-is, or specify NTP servers. `NTP_SERVER_2` and `NTP_SERVER_3` are US based and should be changed to a local server if outside of the US.

#### Syncing
NTP Syncs happen twice a day, once at Noon and once at Midnight. This can be changed by modifying `NTP_TIMER`.

# pxl999
![alt text](https://github.com/dezign999/pxl999/blob/main/pxl999.gif?raw=true?raw=true)

Pxl999 is a chunky pixel watch face for SQFMI's Watchy. This watch face features live weather updates every 30 minutes and NTP syncing twice a day to account for any time drifting.

When internet access is not available, the RTC will be used for ambient temperature, and is indicated by a microchip as the weather icon.

Please make sure to personalize your settings in Watchy_Base.h with your locale, Open Weather Map API Key, NTP servers and timezones. You can get your free Openweathermap api key here: https://openweathermap.org/appid

This watch face was made using the free Arduino IDE, Photoshop CC, image2ccp, Glypher Studio (font creation), Cloud Convert (otf > ttf), and truetype2gfx.

I'm still relatively new to C++ (and rusty), so please excuse the mess. I've taken a lot of David Peer's code from his examples and borrowed much of Symptym's NTP time syncing code.

Please note, that this works on my watchy, your results may vary :P I had a real problem with the watchy crashing if I polled for weather too early at boot, therefore I put a workaround in by forcing a delay and just showing RTC temperature at first boot. If you find a way to fix this, please lmk!

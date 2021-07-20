#ifndef WATCHY_PXL999_H
#define WATCHY_PXL999_H

#include "Watchy_Base.h"
#include "smText8pt7b.h"
#include "timeLG20pt7b.h"
#include "timeLG42pt7b.h"
#include "icons.h"

//extern RTC_DATA_ATTR bool delayedStart;

class WatchyPXL999 : public WatchyBase{
    public:
        WatchyPXL999();
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawWeather();
        void drawCachedWeather();
        void drawWeatherIcon();
        bool pauseUpdates();
};

#endif

#define USE_RTC


#include "lights.h"

#ifdef USE_RTC
#include <Wire.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>



//  Time zone setup
//  US Central Time Zone
TimeChangeRule tzDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule tzST = {"CST", First, dowSunday, Nov, 2, -360};
Timezone tz(tzDT, tzST);
#endif


Lights::Lights(Adafruit_WS2801* pixelObj, bool rtcActive)
{
	_pixelObj = pixelObj;
	_rtcActive = rtcActive;
	if (_pixelObj) {
		_numPixels = _pixelObj->numPixels();
	}
	_lastMillis = 0;
}

Lights::~Lights()
{
}

void
Lights::update()
{
	unsigned long curMillis = millis();

	for (uint8_t i = 0; i < _maxActivePixels; i++) {
		_pixels[i].update();
	}

	if (_lastMillis > curMillis)  _lastMillis = curMillis;
	if (curMillis - _lastMillis > _spawnTime) {
		if (checkTime()) {
			spawnLights();
		}

		_lastMillis = curMillis;
	}
}

bool
Lights::checkTime()
{
	bool rtnVal = true;

#ifdef USE_RTC
	if (_rtcActive) {
		time_t local = tz.toLocal(now());
		int h = hour(local);
		int m = minute(local);

		if (h == _startTimeHour && m < _startTimeMin) {
			rtnVal = false;
		} else if (h == _endTimeHour && m > _endTimeMin) {
			rtnVal = false;
		} else if (_startTimeHour > _endTimeHour) {
			if (h > _endTimeHour && h < _startTimeHour) {
				rtnVal = false;
			}
		} else if (h > _endTimeHour || h < _startTimeHour) {
			rtnVal = false;
		}
	}
#endif

	return rtnVal;
}

void
Lights::modeTransition()
{
	//  Set up the pixels
	for (uint8_t i = 0; i < Lights::maxActivePixels; i++) {
		_pixels[i].reset();
	}


	for (uint8_t i = 0; i < _numPixels; i++) {
		_pixelObj->setPixelColor(i, transitionColors[i]);
	}
	_pixelObj->show();

	if (_rtcActive) {
		delay(200);

		for (uint8_t i = 0; i < _numPixels; i++) {
			_pixelObj->setPixelColor(i, 0x000000);
		}
		_pixelObj->show();
		delay(100);
		for (uint8_t i = 0; i < _numPixels; i++) {
			_pixelObj->setPixelColor(i, transitionColors[i]);
		}
		_pixelObj->show();
		delay(200);
	} else {
		delay(500);
	}

	for (uint8_t i = 0; i < _numPixels; i++) {
		_pixelObj->setPixelColor(i, 0x000000);
	}
	_pixelObj->show();
}


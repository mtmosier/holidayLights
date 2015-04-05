#ifndef LIGHTS_H
#define LIGHTS_H

#include "Arduino.h"
#include "fadePixels.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"



#define NUM_PIXELS                          24


class Lights {
	public:
		static const int maxActivePixels = 10;

		uint32_t transitionColors[NUM_PIXELS];
		Adafruit_WS2801* _pixelObj;
		uint8_t _maxActivePixels;
		uint16_t _numPixels;
		bool _rtcActive;
		int _startTimeHour;
		int _startTimeMin;
		int _endTimeHour;
		int _endTimeMin;
		int _spawnTime;
		unsigned long _lastMillis;

		Lights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~Lights();

		virtual void modeTransition();
		virtual void update();

	protected:
		FadePixels _pixels[maxActivePixels];

		bool checkTime();
		virtual void spawnLights() = 0;
};

#endif // LIGHTS_H
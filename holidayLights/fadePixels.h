#ifndef FADEPIXELS_H
#define FADEPIXELS_H

#include "Arduino.h"
#include "RGB.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class FadePixels {
	public:
		static const int maxPixels = 2;
		static const int maxBrightness = 100;
		static const int stepDelay = 25;
		static const int stepMin = 2;
		static const int stepMax = 8;
		static const int cdMin = 500;
		static const int cdMax = 1250;


		boolean _ready;
		unsigned long _targetMillis;
		Adafruit_WS2801* _pixelObj;

		uint8_t _redMin;
		uint8_t _redMax;
		uint8_t _greenMin;
		uint8_t _greenMax;
		uint8_t _blueMin;
		uint8_t _blueMax;

		int _step;
		int _cooldownTime;
		int _repeatCount;
		boolean _repeatReady;

		RGB _color;

		uint8_t _pixelsToFade[maxPixels];

		FadePixels();
		FadePixels(Adafruit_WS2801* pixelObj);

		void setPixelObj(Adafruit_WS2801* pixelObj);
		void setRedBounds(uint8_t min, uint8_t max);
		void setGreenBounds(uint8_t min, uint8_t max);
		void setBlueBounds(uint8_t min, uint8_t max);
		void reset();
		void repeat();
		boolean start();
		void setPixels(int n, ...);
		void showPixels();
		void update();
		boolean pixelIsAvailable(uint8_t pixelToCheck, uint8_t offsetAllowed);
};

#endif // FADEPIXELS_H

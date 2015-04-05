#include "halloweenLights.h"



HalloweenLights::HalloweenLights(Adafruit_WS2801* pixelObj, bool rtcActive)
: Lights(pixelObj, rtcActive)
{
	for (uint8_t i = 0; i < _numPixels; i++) {
		transitionColors[i] = 0xFF7F00;  //  Orange
	}

	_maxActivePixels = min(3, Lights::maxActivePixels);

	_startTimeHour = 17;
	_startTimeMin = 0;
	_endTimeHour = 7;
	_endTimeMin = 0;
	_maxRepeats = 2;
	_spawnTime = 250;

	for (uint8_t i = 0; i < _maxActivePixels; i++) {
		_pixels[i].setPixelObj(pixelObj);
		_pixels[i].setRedBounds(150, 255);
		_pixels[i].setGreenBounds(0, 100);
		_pixels[i].setBlueBounds(0, 0);
	}
}

HalloweenLights::~HalloweenLights()
{
}

void
HalloweenLights::spawnLights()
{
	for (uint8_t i = 0; i < _maxActivePixels; i++) {
		if (_pixels[i]._ready && _pixels[i]._repeatReady && _pixels[i]._repeatCount < _maxRepeats) {
			//  20% to repeat the first time, 10% the second time
			if (random(0, 5 + (5 * _pixels[i]._repeatCount)) == 1) {
				_pixels[i].repeat();
				_pixels[i].start();
			} else {
				_pixels[i]._repeatReady = false;
			}
		}
	}

	//  10% chance to spawn a new light each time through
	//  Only one light can be spawned at a time (though unlimited repeats can happen at the same moment
	if (random(0, 10) == 1) {
		uint8_t nextLightId = 255;
		for (uint8_t i = 0; i < _maxActivePixels; i++) {
			if (_pixels[i]._ready) {
				nextLightId = i;
				break;
			}
		}

		if (nextLightId != 255) {
			uint8_t newPos = 255;

			while (newPos == 255) {
				newPos = random(0, _numPixels / 2) * 2;

				for (uint8_t i = 0; i < _maxActivePixels; i++) {
					if (!_pixels[i].pixelIsAvailable(newPos, 2)) {
						newPos = 255;
						break;
					}
				}
			}

			_pixels[nextLightId].reset();
			_pixels[nextLightId].setPixels(2, newPos, newPos + 1);
			_pixels[nextLightId].start();
		}
	}
}

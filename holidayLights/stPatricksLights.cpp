#include "stPatricksLights.h"



StPatricksLights::StPatricksLights(Adafruit_WS2801* pixelObj, bool rtcActive)
: Lights(pixelObj, rtcActive)
{
	for (uint8_t i = 0; i < _numPixels; i++) {
		transitionColors[i] = 0x00FF00;  //  Green
	}

	_maxActivePixels = min(8, Lights::maxActivePixels);

	_startTimeHour = 8;
	_startTimeMin = 0;
	_endTimeHour = 1;
	_endTimeMin = 0;
	_spawnTime = 75;

	for (uint8_t i = 0; i < _maxActivePixels; i++) {
		_pixels[i].setPixelObj(pixelObj);
		_pixels[i].setRedBounds(0, 75);
		_pixels[i].setGreenBounds(200, 255);
		_pixels[i].setBlueBounds(0, 75);
	}
}

StPatricksLights::~StPatricksLights()
{
}

void
StPatricksLights::spawnLights()
{
	uint8_t color = 0;

	//  20% chance to spawn a new light each time through
	//  Only one light can be spawned at a time
	if (random(0, 5) == 1) {
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
					if (!_pixels[i].pixelIsAvailable(newPos, 1)) {
						newPos = 255;
						break;
					}
				}
			}

			_pixels[nextLightId].reset();
			_pixels[nextLightId].setPixels(1, newPos);
			_pixels[nextLightId].start();
		}
	}
}

#include "greenAndGoldLights.h"



GreenAndGoldLights::GreenAndGoldLights(Adafruit_WS2801* pixelObj, bool rtcActive)
: Lights(pixelObj, rtcActive)
{
	for (uint8_t i = 0; i < _numPixels; i++) {
		switch (i % 2) {
			case 0:
				transitionColors[i] = 0x00FF00;  //  Green
				break;
			case 1:
				transitionColors[i] = 0xFFD700;  //  Gold
				break;
		}
	}

	_maxActivePixels = min(8, Lights::maxActivePixels);

	_startTimeHour = 8;
	_startTimeMin = 0;
	_endTimeHour = 1;
	_endTimeMin = 0;
	_spawnTime = 75;

	for (uint8_t i = 0; i < _maxActivePixels; i++) {
		_pixels[i].setPixelObj(pixelObj);
	}
}

GreenAndGoldLights::~GreenAndGoldLights()
{
}

void
GreenAndGoldLights::spawnLights()
{
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


			switch (random(0, 2)) {
				case 0:
					_pixels[nextLightId].setRedBounds(0, 0);
					_pixels[nextLightId].setGreenBounds(255, 255);
					_pixels[nextLightId].setBlueBounds(0, 0);
					break;
				case 1:
					_pixels[nextLightId].setRedBounds(255, 255);
					_pixels[nextLightId].setGreenBounds(215, 215);
					_pixels[nextLightId].setBlueBounds(0, 0);
					break;
			}

			_pixels[nextLightId].reset();
			_pixels[nextLightId].setPixels(1, newPos);
			_pixels[nextLightId].start();
		}
	}
}

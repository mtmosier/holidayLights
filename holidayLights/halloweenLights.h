#ifndef HALLOWEEN_LIGHTS_H
#define HALLOWEEN_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class HalloweenLights : public Lights {
	public:
		HalloweenLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~HalloweenLights();

	protected:
		int _maxRepeats;

		void spawnLights();
};

#endif // HALLOWEEN_LIGHTS_H

#ifndef GREEN_AND_GOLD_LIGHTS_H
#define GREEN_AND_GOLD_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class GreenAndGoldLights : public Lights {
	public:
		GreenAndGoldLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~GreenAndGoldLights();

	protected:
		void spawnLights();
};

#endif // GREEN_AND_GOLD_LIGHTS_H

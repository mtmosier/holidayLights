#ifndef ST_PATRICKS_LIGHTS_H
#define ST_PATRICKS_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class StPatricksLights : public Lights {
	public:
		StPatricksLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~StPatricksLights();

	protected:
		void spawnLights();
};

#endif // ST_PATRICKS_LIGHTS_H

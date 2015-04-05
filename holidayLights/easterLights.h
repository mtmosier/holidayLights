#ifndef EASTER_LIGHTS_H
#define EASTER_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class EasterLights : public Lights {
	public:
		EasterLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~EasterLights();

	protected:
		void spawnLights();
};

#endif // EASTER_LIGHTS_H

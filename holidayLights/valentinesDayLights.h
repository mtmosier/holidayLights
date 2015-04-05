#ifndef VALENTINES_LIGHTS_H
#define VALENTINES_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class ValentinesDayLights : public Lights {
	public:
		ValentinesDayLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~ValentinesDayLights();

	protected:
		void spawnLights();
};

#endif // VALENTINES_LIGHTS_H

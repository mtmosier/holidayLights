#ifndef CHISTMAS_LIGHTS_H
#define CHISTMAS_LIGHTS_H

#include "Arduino.h"
#include "lights.h"
#include "SPI.h"
#include "Adafruit_WS2801.h"


class ChistmasLights : public Lights {
	public:
		ChistmasLights(Adafruit_WS2801* pixelObj, bool rtcActive);
		virtual ~ChistmasLights();

	protected:
		int _maxRepeats;

		void spawnLights();
};

#endif // CHISTMAS_LIGHTS_H

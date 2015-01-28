#define USE_RTC

#include "SPI.h"
#include "Adafruit_WS2801.h"
#include <EEPROM.h>
#ifdef USE_RTC
#include <Wire.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>
#endif


#define DATA_PIN                             4
#define CLOCK_PIN                            3
#define NUM_PIXELS                          24
#define MAX_ACTIVE_HALLOWEEN_EYES            3
#define MAX_ACTIVE_CHRISTMAS_LIGHTS          8
#define SPAWN_TIME_EYES                    250
#define SPAWN_TIME_XMAS                     75
#define MAX_REPEATS                          2
#define MODE_ADDR                            0  // EEPROM address for the mode value
#define INT_NUMBER                           0  // Interrupt to use for the button (0 = digital pin 2)
#define MAX_ACTIVE_PIXELS                    (MAX_ACTIVE_CHRISTMAS_LIGHTS > MAX_ACTIVE_HALLOWEEN_EYES ? MAX_ACTIVE_CHRISTMAS_LIGHTS : MAX_ACTIVE_HALLOWEEN_EYES)
#define DEBOUNCE_TIME                      150

#ifdef USE_RTC
#define START_TIME_HALLOWEEN_EYES_HOUR      17
#define START_TIME_HALLOWEEN_EYES_MINUTE     0
#define END_TIME_HALLOWEEN_EYES_HOUR         7
#define END_TIME_HALLOWEEN_EYES_MINUTE       0
#define START_TIME_CHRISTMAS_LIGHTS_HOUR     8
#define START_TIME_CHRISTMAS_LIGHTS_MINUTE   0
#define END_TIME_CHRISTMAS_LIGHTS_HOUR       1
#define END_TIME_CHRISTMAS_LIGHTS_MINUTE     0


//  Time zone setup
//  US Central Time Zone
TimeChangeRule tzDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule tzST = {"CST", First, dowSunday, Nov, 2, -360};
Timezone tz(tzDT, tzST);

DS3231 clock;
#endif



Adafruit_WS2801 strip = Adafruit_WS2801(NUM_PIXELS, DATA_PIN, CLOCK_PIN);



volatile int mode = 0;
volatile boolean updateMode = false;
volatile unsigned long lastDebounceTime = 0;
unsigned long lastMillis = 0;






void spawnHalloweenEyes();
void spawnChistmasLights();
boolean checkModeTime();
void modeTransition();
void buttonPress();


class RGB {
	public:
		uint8_t _r;
		uint8_t _g;
		uint8_t _b;
		int _intensity;


		RGB()
		{
			_r = 0;
			_g = 0;
			_b = 0;
			_intensity = 255;
		}

		RGB(uint8_t r, uint8_t g, uint8_t b)
		{
			_r = r;
			_g = g;
			_b = b;
			_intensity = 255;
		}

		uint32_t
		getColor()
		{
			uint32_t c;

			c = map(_r, 0, 255, 0, _intensity);
			c <<= 8;
			c |= map(_g, 0, 255, 0, _intensity);
			c <<= 8;
			c |= map(_b, 0, 255, 0, _intensity);

			return c;
		}

		friend bool
		operator== (RGB &c1, RGB &c2)
		{
			return (c1._r == c2._r && c1._g == c2._g && c1._b == c2._b);
		}

		friend bool
		operator!= (RGB &c1, RGB &c2)
		{
			return !(c1 == c2);
		}

		void
		operator=(const RGB &co)
		{
			_r = co._r;
			_g = co._g;
			_b = co._b;
			_intensity = co._intensity;
		}
};


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

		FadePixels()
		{
			_pixelObj = NULL;
			_ready = true;
			_repeatCount = 0;
			_repeatReady = false;
			_color = RGB();

			setRedBounds(0, 255);
			setGreenBounds(0, 255);
			setBlueBounds(0, 255);
		}

		FadePixels(Adafruit_WS2801* pixelObj)
		{
			_pixelObj = pixelObj;
			_ready = true;
			_repeatCount = 0;
			_repeatReady = false;
			_color = RGB();

			setRedBounds(0, 255);
			setGreenBounds(0, 255);
			setBlueBounds(0, 255);
		}

		void
		setPixelObj(Adafruit_WS2801* pixelObj)
		{
			_pixelObj = pixelObj;
		}

		void
		setRedBounds(uint8_t min, uint8_t max)
		{
			_redMin = min;
			_redMax = max;
		}

		void
		setGreenBounds(uint8_t min, uint8_t max)
		{
			_greenMin = min;
			_greenMax = max;
		}

		void
		setBlueBounds(uint8_t min, uint8_t max)
		{
			_blueMin = min;
			_blueMax = max;
		}

		void
		reset()
		{
			_ready = true;
			_repeatCount = 0;
			_repeatReady = false;

			_color._r = random(_redMin, _redMax);
			_color._g = random(_greenMin, _greenMax);
			_color._b = random(_blueMin, _blueMax);
			_color._intensity = 0;
			_step = random(stepMin, stepMax);
			_cooldownTime = random(cdMin, cdMax);
			showPixels();

			for (uint8_t i = 0; i < maxPixels; i++) {
				_pixelsToFade[i] = 255;
			}

			_targetMillis = millis();
		}

		void
		repeat()
		{
			_ready = true;
			_repeatReady = false;
			_color._intensity = 0;
			_repeatCount++;
			showPixels();

			_targetMillis = millis();
		}

		boolean
		start()
		{
			uint8_t i;
			boolean readyToStart = true;

			if (!_pixelObj) {
				readyToStart = false;
			}

			if (_pixelsToFade[0] == 255) {
				readyToStart = false;
			}

			if (_color._r == 0 && _color._g == 0 && _color._b == 0) {
				readyToStart = false;
			}

			if (readyToStart) {
				_ready = false;
			} else {
				_ready = true;
			}

			return readyToStart;
		}

		void
		setPixels(int n, ...)
		{
			int pixelId;
			va_list arguments;

			va_start(arguments, n);

			for (uint8_t i = 0; i < n; i++) {
				pixelId = va_arg(arguments, int);
				_pixelsToFade[i] = pixelId;
			}
			va_end(arguments);
		}

		void
		showPixels()
		{
			if (!_pixelObj) {
				return;
			}

			for (uint8_t i = 0; i < maxPixels; i++) {
				if (_pixelsToFade[i] == 255) {
					break;
				}

				_pixelObj->setPixelColor(_pixelsToFade[i], _color.getColor());
			}
			_pixelObj->show();
		}

		void
		update()
		{
			if (_ready) {  //  We're currently waiting to be used again
				return;
			}

			if (millis() < _targetMillis) {  //  It's not time yet!!
				return;
			}

			if (_color._intensity == 0 && _step < 0) {
				_step *= -1;
				_ready = true;
				_repeatReady = true;
			} else {
				_color._intensity += _step;
				_targetMillis = millis() + stepDelay;

				if (_step > 0) {
					if (_color._intensity >= maxBrightness) {
						_color._intensity -= _step;
						_step *= -1;
					}
				} else {
					if (_color._intensity < 0) {
						_color._intensity = 0;
						_targetMillis = millis() + _cooldownTime;
					}
				}

				showPixels();
			}
		}

		boolean
		pixelIsAvailable(uint8_t pixelToCheck, uint8_t offsetAllowed)
		{
			boolean isAvailable = true;

			for (uint8_t i = 0; i < maxPixels; i++) {
				if (_pixelsToFade[i] == 255) {
					break;
				}

				if (abs(_pixelsToFade[i] - pixelToCheck) <= offsetAllowed) {
					isAvailable = false;
					break;
				}
			}

			return isAvailable;
		}
};


FadePixels pixels[MAX_ACTIVE_PIXELS];

void setup() 
{
	randomSeed(analogRead(0));

#ifdef USE_RTC
	Wire.begin();
	clock.setClockMode(false);

	byte yr, mo, day, dow, h, m, s;
	clock.getTime(yr, mo, day, dow, h, m, s);
	setTime(h, m, s, day, mo, yr);
#endif

	mode = EEPROM.read(MODE_ADDR);
#ifdef USE_RTC
	if (mode > 3) {
#else
	if (mode > 1) {
#endif
		mode = 0;
	}

	attachInterrupt(INT_NUMBER, buttonPress, RISING);


	// initialize the strip
	strip.begin();
	strip.show();

	for (uint8_t i = 0; i < MAX_ACTIVE_PIXELS; i++) {
		pixels[i].setPixelObj(&strip);
		if ((mode == 0 || mode == 2) && i < MAX_ACTIVE_HALLOWEEN_EYES) {
			pixels[i].setRedBounds(150, 255);
			pixels[i].setGreenBounds(0, 100);
			pixels[i].setBlueBounds(0, 0);
		}
	}
}


void loop()
{
	if (updateMode) {
		modeTransition();
	}

	for (uint8_t i = 0; i < MAX_ACTIVE_PIXELS; i++) {
		pixels[i].update();
	}

	if (mode == 0 || mode == 2) {
		spawnHalloweenEyes();
	} else {
		spawnChistmasLights();
	}
}


void
spawnHalloweenEyes()
{
	if (lastMillis > millis())  lastMillis = millis();
	if (millis() - lastMillis > SPAWN_TIME_EYES) {
		if (checkModeTime()) {
			for (uint8_t i = 0; i < MAX_ACTIVE_HALLOWEEN_EYES; i++) {
				if (pixels[i]._ready && pixels[i]._repeatReady && pixels[i]._repeatCount < MAX_REPEATS) {
					//  20% to repeat the first time, 10% the second time
					if (random(0, 5 + (5 * pixels[i]._repeatCount)) == 1) {
						pixels[i].repeat();
						pixels[i].start();
					} else {
						pixels[i]._repeatReady = false;
					}
				}
			}

			//  10% chance to spawn a new light each time through
			//  Only one light can be spawned at a time (though unlimited repeats can happen at the same moment
			if (random(0, 10) == 1) {
				uint8_t nextLightId = 255;
				for (uint8_t i = 0; i < MAX_ACTIVE_HALLOWEEN_EYES; i++) {
					if (pixels[i]._ready) {
						nextLightId = i;
						break;
					}
				}

				if (nextLightId != 255) {
					uint8_t newPos = 255;

					while (newPos == 255) {
						newPos = random(0, NUM_PIXELS / 2) * 2;

						for (uint8_t i = 0; i < MAX_ACTIVE_HALLOWEEN_EYES; i++) {
							if (!pixels[i].pixelIsAvailable(newPos, 2)) {
								newPos = 255;
								break;
							}
						}
					}

					pixels[nextLightId].reset();
					pixels[nextLightId].setPixels(2, newPos, newPos + 1);
					pixels[nextLightId].start();
				}
			}
		}

		lastMillis = millis();
	}
}


void
spawnChistmasLights()
{
	if (lastMillis > millis())  lastMillis = millis();
	if (millis() - lastMillis > SPAWN_TIME_XMAS) {
		if (checkModeTime()) {
			//  20% chance to spawn a new light each time through
			//  Only one light can be spawned at a time
			if (random(0, 5) == 1) {
				uint8_t nextLightId = 255;
				for (uint8_t i = 0; i < MAX_ACTIVE_CHRISTMAS_LIGHTS; i++) {
					if (pixels[i]._ready) {
						nextLightId = i;
						break;
					}
				}

				if (nextLightId != 255) {
					uint8_t newPos = 255;

					while (newPos == 255) {
						newPos = random(0, NUM_PIXELS / 2) * 2;

						for (uint8_t i = 0; i < MAX_ACTIVE_CHRISTMAS_LIGHTS; i++) {
							if (!pixels[i].pixelIsAvailable(newPos, 1)) {
								newPos = 255;
								break;
							}
						}
					}


					switch (random(0, 3)) {
						case 0:
							pixels[nextLightId].setRedBounds(255, 255);
							pixels[nextLightId].setGreenBounds(0, 0);
							pixels[nextLightId].setBlueBounds(0, 0);
							break;
						case 1:
							pixels[nextLightId].setRedBounds(255, 255);
							pixels[nextLightId].setGreenBounds(255, 255);
							pixels[nextLightId].setBlueBounds(255, 255);
							break;
						case 2:
							pixels[nextLightId].setRedBounds(0, 0);
							pixels[nextLightId].setGreenBounds(255, 255);
							pixels[nextLightId].setBlueBounds(0, 0);
							break;
					}

					pixels[nextLightId].reset();
					pixels[nextLightId].setPixels(1, newPos);
					pixels[nextLightId].start();
				}
			}
		}

		lastMillis = millis();
	}
}


boolean
checkModeTime()
{
	boolean rtnVal = true;

#ifdef USE_RTC
	if (mode == 2) {
		time_t local = tz.toLocal(now());
		int h = hour(local);
		int m = minute(local);

		if (h == START_TIME_HALLOWEEN_EYES_HOUR && m < START_TIME_HALLOWEEN_EYES_MINUTE) {
			rtnVal = false;
		} else if (h == END_TIME_HALLOWEEN_EYES_HOUR && m > END_TIME_HALLOWEEN_EYES_MINUTE) {
			rtnVal = false;
		} else if (START_TIME_HALLOWEEN_EYES_HOUR > END_TIME_HALLOWEEN_EYES_HOUR) {
			if (h > END_TIME_HALLOWEEN_EYES_HOUR && h < START_TIME_HALLOWEEN_EYES_HOUR) {
				rtnVal = false;
			}
		} else if (h > END_TIME_HALLOWEEN_EYES_HOUR || h < START_TIME_HALLOWEEN_EYES_HOUR) {
			rtnVal = false;
		}

	} else if (mode == 3) {
		time_t local = tz.toLocal(now());
		int h = hour(local);
		int m = minute(local);

		if (h == START_TIME_CHRISTMAS_LIGHTS_HOUR && m < START_TIME_CHRISTMAS_LIGHTS_MINUTE) {
			rtnVal = false;
		} else if (h == END_TIME_CHRISTMAS_LIGHTS_HOUR && m > END_TIME_CHRISTMAS_LIGHTS_MINUTE) {
			rtnVal = false;
		} else if (START_TIME_CHRISTMAS_LIGHTS_HOUR > END_TIME_CHRISTMAS_LIGHTS_HOUR) {
			if (h > END_TIME_CHRISTMAS_LIGHTS_HOUR && h < START_TIME_CHRISTMAS_LIGHTS_HOUR) {
				rtnVal = false;
			}
		} else if (h > END_TIME_CHRISTMAS_LIGHTS_HOUR || h < START_TIME_CHRISTMAS_LIGHTS_HOUR) {
			rtnVal = false;
		}
	}
#endif

	return rtnVal;
}


void
modeTransition()
{
	updateMode = false;

	for (uint8_t i = 0; i < MAX_ACTIVE_PIXELS; i++) {
		pixels[i].reset();
	}

	if (mode == 0 || mode == 2) {
		for (uint8_t i = 0; i < MAX_ACTIVE_HALLOWEEN_EYES; i++) {
			pixels[i].setRedBounds(150, 255);
			pixels[i].setGreenBounds(0, 100);
			pixels[i].setBlueBounds(0, 0);
		}

		for (uint8_t i = 0; i < NUM_PIXELS; i++) {
			strip.setPixelColor(i, 0xFF7F00);
		}
		strip.show();

		if (mode == 2) {
			delay(200);

			for (uint8_t i = 0; i < NUM_PIXELS; i++) {
				strip.setPixelColor(i, 0x000000);
			}
			strip.show();
			delay(100);
			for (uint8_t i = 0; i < NUM_PIXELS; i++) {
				strip.setPixelColor(i, 0xFF7F00);
			}
			strip.show();
			delay(200);
		} else {
			delay(500);
		}

	} else {
		for (uint8_t i = 0; i < NUM_PIXELS; i++) {
			if (i % 2 == 1) {
				strip.setPixelColor(i, 0x00FF00);
			} else {
				strip.setPixelColor(i, 0xFF0000);
			}
		}
		strip.show();

		if (mode == 3) {
			delay(200);

			for (uint8_t i = 0; i < NUM_PIXELS; i++) {
				strip.setPixelColor(i, 0x000000);
			}
			strip.show();
			delay(100);
			for (uint8_t i = 0; i < NUM_PIXELS; i++) {
				if (i % 2 == 1) {
					strip.setPixelColor(i, 0xFF0000);
				} else {
					strip.setPixelColor(i, 0x00FF00);
				}
			}
			strip.show();
			delay(200);
		} else {
			delay(500);
		}
	}

	for (uint8_t i = 0; i < NUM_PIXELS; i++) {
		strip.setPixelColor(i, 0x000000);
	}
	strip.show();
}


void
buttonPress()
{
	if (lastDebounceTime > millis())  lastDebounceTime = millis();
	if (millis() - lastDebounceTime > DEBOUNCE_TIME) {
		updateMode = true;
		mode++;
#ifdef USE_RTC
		if (mode > 3) {
#else
		if (mode > 1) {
#endif
			mode = 0;
		}

		EEPROM.write(MODE_ADDR, mode);
		lastDebounceTime = millis();
	}
}

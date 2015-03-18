#define USE_RTC

#include "SPI.h"
#include "Adafruit_WS2801.h"
#include "RGB.h"
#include "fadePixels.h"
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
#define MAX_ACTIVE_GENERAL_LIGHTS            8
#define MAX_ACTIVE_PIXELS                    8  // Set to the highest of the above values (for halloween multiply by 2)
#define SPAWN_TIME_EYES                    250
#define SPAWN_TIME_GENERAL                  75
#define MAX_REPEATS                          2
#define MODE_ADDR                            0  // EEPROM address for the mode value
#define RTC_ACTIVE_ADDR                      1  // EEPROM address for the rtc active value
#define INT_NUMBER                           0  // Interrupt to use for the button (0 = digital pin 2)
#define BUTTON_PIN                           2  // Tied to interrupt above.  Make sure these line up
#define DEBOUNCE_TIME                      200
#define RTC_CHANGE_TIME                   2000  //  Time to hold the button to switch into timed mode
#define MODE_COUNT                           4

#ifdef USE_RTC
#define START_TIME_HALLOWEEN_EYES_HOUR      17
#define START_TIME_HALLOWEEN_EYES_MINUTE     0
#define END_TIME_HALLOWEEN_EYES_HOUR         7
#define END_TIME_HALLOWEEN_EYES_MINUTE       0
#define START_TIME_GENERAL_HOUR              8
#define START_TIME_GENERAL_MINUTE            0
#define END_TIME_GENERAL_HOUR                1
#define END_TIME_GENERAL_MINUTE              0


//  Time zone setup
//  US Central Time Zone
TimeChangeRule tzDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule tzST = {"CST", First, dowSunday, Nov, 2, -360};
Timezone tz(tzDT, tzST);

DS3231 clock;
#endif


typedef struct
{
	uint8_t mode;
	uint32_t colors[NUM_PIXELS];
} ColorMap;


const ColorMap transitionColors[MODE_COUNT] = {
	{ 0,
		{ 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00,
		  0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00,
		  0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00,
		  0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00, 0xFF7F00
		}
	},
	{ 1,
		{ 0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000,
		  0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000,
		  0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000,
		  0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000, 0x00FF00, 0xFF0000
		}
	},
	{ 2,
		{ 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFF0000, 0xFFFFFF, 0xFFFFFF,
		  0xFFFFFF, 0xFFFFFF, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
		  0xFFFFFF, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
		  0xFFFFFF, 0xFF0000, 0xFFFFFF, 0xFFFFFF, 0xFF0000, 0xFFFFFF
		}
	},
	{ 3,
		{ 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00,
		  0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00,
		  0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00,
		  0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00
		}
	}
};



volatile int mode = 0;
volatile boolean rtcActive = false;
volatile boolean updateMode = false;
volatile int lastButtonState = LOW;
volatile int buttonState = LOW;
volatile unsigned long lastDebounceTime = 0;
unsigned long lastMillis = 0;


Adafruit_WS2801 strip = Adafruit_WS2801(NUM_PIXELS, DATA_PIN, CLOCK_PIN);

FadePixels pixels[MAX_ACTIVE_PIXELS];




//  Not having these definitions here causes errors when compiling without rtc support
void buttonPress();
boolean checkModeTime();
void modeTransition();
void spawnHalloweenEyes();
void spawnChistmasLights();
void spawnValentinesDayLights();
void spawnStPatricksLights();



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
	if (mode >= MODE_COUNT) {
		mode = 0;
	}

#ifdef USE_RTC
	rtcActive = (EEPROM.read(RTC_ACTIVE_ADDR) == 1);
#else
	rtcActive = false;
#endif

	attachInterrupt(INT_NUMBER, buttonPress, CHANGE);


	// initialize the strip
	strip.begin();
	strip.show();

	for (uint8_t i = 0; i < MAX_ACTIVE_PIXELS; i++) {
		pixels[i].setPixelObj(&strip);
		if (mode == 0 && i < MAX_ACTIVE_HALLOWEEN_EYES) {
			pixels[i].setRedBounds(150, 255);
			pixels[i].setGreenBounds(0, 100);
			pixels[i].setBlueBounds(0, 0);
		} else if (mode == 2 && i < MAX_ACTIVE_GENERAL_LIGHTS) {
			pixels[i].setRedBounds(255, 255);
		} else if (mode == 3 && i < MAX_ACTIVE_GENERAL_LIGHTS) {
			pixels[i].setRedBounds(0, 75);
			pixels[i].setGreenBounds(200, 255);
			pixels[i].setBlueBounds(0, 75);
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

	if (mode == 0) {
		spawnHalloweenEyes();
	} else if (mode == 1) {
		spawnChistmasLights();
	} else if (mode == 2) {
		spawnValentinesDayLights();
	} else if (mode == 3) {
		spawnStPatricksLights();
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
	if (millis() - lastMillis > SPAWN_TIME_GENERAL) {
		if (checkModeTime()) {
			//  20% chance to spawn a new light each time through
			//  Only one light can be spawned at a time
			if (random(0, 5) == 1) {
				uint8_t nextLightId = 255;
				for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
					if (pixels[i]._ready) {
						nextLightId = i;
						break;
					}
				}

				if (nextLightId != 255) {
					uint8_t newPos = 255;

					while (newPos == 255) {
						newPos = random(0, NUM_PIXELS / 2) * 2;

						for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
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


void
spawnValentinesDayLights()
{
	uint8_t color = 0;

	if (lastMillis > millis())  lastMillis = millis();
	if (millis() - lastMillis > SPAWN_TIME_GENERAL) {
		if (checkModeTime()) {
			//  20% chance to spawn a new light each time through
			//  Only one light can be spawned at a time
			if (random(0, 5) == 1) {
				uint8_t nextLightId = 255;
				for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
					if (pixels[i]._ready) {
						nextLightId = i;
						break;
					}
				}

				if (nextLightId != 255) {
					uint8_t newPos = 255;

					while (newPos == 255) {
						newPos = random(0, NUM_PIXELS / 2) * 2;

						for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
							if (!pixels[i].pixelIsAvailable(newPos, 1)) {
								newPos = 255;
								break;
							}
						}
					}

					//  Up the chances of red being chosen
					if (random(0, 5) == 0) {
						color = 0;
					} else {
						color = random(0, 200);
					}

					pixels[nextLightId].setGreenBounds(color, color);
					pixels[nextLightId].setBlueBounds(color, color);
					pixels[nextLightId].reset();
					pixels[nextLightId].setPixels(1, newPos);
					pixels[nextLightId].start();
				}
			}
		}

		lastMillis = millis();
	}
}


void
spawnStPatricksLights()
{
	if (lastMillis > millis())  lastMillis = millis();
	if (millis() - lastMillis > SPAWN_TIME_GENERAL) {
		if (checkModeTime()) {
			//  20% chance to spawn a new light each time through
			//  Only one light can be spawned at a time
			if (random(0, 5) == 1) {
				uint8_t nextLightId = 255;
				for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
					if (pixels[i]._ready) {
						nextLightId = i;
						break;
					}
				}

				if (nextLightId != 255) {
					uint8_t newPos = 255;

					while (newPos == 255) {
						newPos = random(0, NUM_PIXELS / 2) * 2;

						for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
							if (!pixels[i].pixelIsAvailable(newPos, 1)) {
								newPos = 255;
								break;
							}
						}
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
	if (rtcActive) {
		time_t local = tz.toLocal(now());
		int h = hour(local);
		int m = minute(local);

		if (mode == 0) {
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

		} else {
			//  General case, for any modes without a specific setting
			if (h == START_TIME_GENERAL_HOUR && m < START_TIME_GENERAL_MINUTE) {
				rtnVal = false;
			} else if (h == END_TIME_GENERAL_HOUR && m > END_TIME_GENERAL_MINUTE) {
				rtnVal = false;
			} else if (START_TIME_GENERAL_HOUR > END_TIME_GENERAL_HOUR) {
				if (h > END_TIME_GENERAL_HOUR && h < START_TIME_GENERAL_HOUR) {
					rtnVal = false;
				}
			} else if (h > END_TIME_GENERAL_HOUR || h < START_TIME_GENERAL_HOUR) {
				rtnVal = false;
			}
		}
	}
#endif

	return rtnVal;
}


void
modeTransition()
{
	updateMode = false;

	//  Set up the pixels
	for (uint8_t i = 0; i < MAX_ACTIVE_PIXELS; i++) {
		pixels[i].reset();
	}

	if (mode == 0) {
		for (uint8_t i = 0; i < MAX_ACTIVE_HALLOWEEN_EYES; i++) {
			pixels[i].setRedBounds(150, 255);
			pixels[i].setGreenBounds(0, 100);
			pixels[i].setBlueBounds(0, 0);
		}

	} else if (mode == 2) {
		for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
			pixels[i].setRedBounds(255, 255);
		}

	} else if (mode == 3) {
		for (uint8_t i = 0; i < MAX_ACTIVE_GENERAL_LIGHTS; i++) {
			pixels[i].setRedBounds(0, 75);
			pixels[i].setGreenBounds(200, 255);
			pixels[i].setBlueBounds(0, 75);
		}
	}


	//  Display the mode transition
	for (uint8_t i = 0; i < MODE_COUNT; i++) {
		if (transitionColors[i].mode == mode) {
			for (uint8_t j = 0; j < NUM_PIXELS; j++) {
				strip.setPixelColor(j, transitionColors[i].colors[j]);
			}
			strip.show();

			if (rtcActive) {
				delay(200);

				for (uint8_t j = 0; j < NUM_PIXELS; j++) {
					strip.setPixelColor(j, 0x000000);
				}
				strip.show();
				delay(100);
				for (uint8_t j = 0; j < NUM_PIXELS; j++) {
					strip.setPixelColor(j, transitionColors[i].colors[j]);
				}
				strip.show();
				delay(200);
			} else {
				delay(500);
			}

			break;
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
	unsigned long curTime = millis();
	int curButtonState = digitalRead(BUTTON_PIN);

	if (lastDebounceTime > curTime)  lastDebounceTime = curTime;

	if (curTime - lastDebounceTime > DEBOUNCE_TIME) {
		if (lastButtonState == HIGH) {
#ifdef USE_RTC
			if (curTime - lastDebounceTime > RTC_CHANGE_TIME) {
				rtcActive = rtcActive ? false : true;
				updateMode = true;
			} else {
#endif
				updateMode = true;
				mode++;

				if (mode >= MODE_COUNT) {
					mode = 0;
				}
#ifdef USE_RTC
			}
#endif
		}

		if (updateMode) {
			EEPROM.write(MODE_ADDR, mode);
			EEPROM.write(RTC_ACTIVE_ADDR, rtcActive ? 1 : 0);
		}
	}

	lastButtonState = curButtonState;
	lastDebounceTime = curTime;
}


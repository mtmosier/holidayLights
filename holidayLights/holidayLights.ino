#define USE_RTC

#include "SPI.h"
#include "Adafruit_WS2801.h"
#include "RGB.h"
#include "halloweenLights.h"
#include "chistmasLights.h"
#include "valentinesDayLights.h"
#include "stPatricksLights.h"
#include "easterLights.h"
#include "greenAndGoldLights.h"
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
#define MODE_ADDR                            0  // EEPROM address for the mode value
#define RTC_ACTIVE_ADDR                      1  // EEPROM address for the rtc active value
#define INT_NUMBER                           0  // Interrupt to use for the button (0 = digital pin 2)
#define BUTTON_PIN                           2  // Tied to interrupt above.  Make sure these line up
#define DEBOUNCE_TIME                      200
#define RTC_CHANGE_TIME                   2000  //  Time to hold the button to switch into timed mode
#define MODE_COUNT                           6



volatile int mode = 0;
volatile bool rtcActive = false;
volatile bool updateMode = false;
volatile int lastButtonState = LOW;
volatile int buttonState = LOW;
volatile unsigned long lastDebounceTime = 0;
unsigned long lastMillis = 0;
Lights* lightDecorationObj = NULL;

#ifdef USE_RTC
DS3231 clock;
#endif
Adafruit_WS2801 strip = Adafruit_WS2801(NUM_PIXELS, DATA_PIN, CLOCK_PIN);


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

	setLightDecorationObject();
}


void loop()
{
	if (updateMode) {
		delete lightDecorationObj;
		setLightDecorationObject();

		if (lightDecorationObj) {
			lightDecorationObj->modeTransition();
		}
		updateMode = false;
	}

	if (lightDecorationObj) {
		lightDecorationObj->update();
	}
}


void
setLightDecorationObject() {
	lightDecorationObj = NULL;

	switch (mode) {
		case 0:
			lightDecorationObj = new HalloweenLights(&strip, (rtcActive == true));
			break;
		case 1:
			lightDecorationObj = new ChistmasLights(&strip, (rtcActive == true));
			break;
		case 2:
			lightDecorationObj = new ValentinesDayLights(&strip, (rtcActive == true));
			break;
		case 3:
			lightDecorationObj = new StPatricksLights(&strip, (rtcActive == true));
			break;
		case 4:
			lightDecorationObj = new EasterLights(&strip, (rtcActive == true));
			break;
		case 5:
			lightDecorationObj = new GreenAndGoldLights(&strip, (rtcActive == true));
			break;
	}
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


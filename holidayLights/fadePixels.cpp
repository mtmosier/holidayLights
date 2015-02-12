#include "fadePixels.h"


const int FadePixels::maxPixels;
const int FadePixels::maxBrightness;
const int FadePixels::stepDelay;
const int FadePixels::stepMin;
const int FadePixels::stepMax;
const int FadePixels::cdMin;
const int FadePixels::cdMax;


FadePixels::FadePixels()
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

FadePixels::FadePixels(Adafruit_WS2801* pixelObj)
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
FadePixels::setPixelObj(Adafruit_WS2801* pixelObj)
{
	_pixelObj = pixelObj;
}

void
FadePixels::setRedBounds(uint8_t min, uint8_t max)
{
	_redMin = min;
	_redMax = max;
}

void
FadePixels::setGreenBounds(uint8_t min, uint8_t max)
{
	_greenMin = min;
	_greenMax = max;
}

void
FadePixels::setBlueBounds(uint8_t min, uint8_t max)
{
	_blueMin = min;
	_blueMax = max;
}

void
FadePixels::reset()
{
	_ready = true;
	_repeatCount = 0;
	_repeatReady = false;

	_color._r = random(_redMin, _redMax + 1);
	_color._g = random(_greenMin, _greenMax + 1);
	_color._b = random(_blueMin, _blueMax + 1);
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
FadePixels::repeat()
{
	_ready = true;
	_repeatReady = false;
	_color._intensity = 0;
	_repeatCount++;
	showPixels();

	_targetMillis = millis();
}

boolean
FadePixels::start()
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
FadePixels::setPixels(int n, ...)
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
FadePixels::showPixels()
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
FadePixels::update()
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
FadePixels::pixelIsAvailable(uint8_t pixelToCheck, uint8_t offsetAllowed)
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


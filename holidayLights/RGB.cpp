#include "RGB.h"


RGB::RGB()
{
	_r = 0;
	_g = 0;
	_b = 0;
	_intensity = 255;
}

RGB::RGB(uint8_t r, uint8_t g, uint8_t b)
{
	_r = r;
	_g = g;
	_b = b;
	_intensity = 255;
}

uint32_t
RGB::getColor()
{
	uint32_t c;

	c = map(_r, 0, 255, 0, _intensity);
	c <<= 8;
	c |= map(_g, 0, 255, 0, _intensity);
	c <<= 8;
	c |= map(_b, 0, 255, 0, _intensity);

	return c;
}

bool
operator== (RGB &c1, RGB &c2)
{
	return (c1._r == c2._r && c1._g == c2._g && c1._b == c2._b);
}

bool
operator!= (RGB &c1, RGB &c2)
{
	return !(c1 == c2);
}

void
RGB::operator=(const RGB &co)
{
	_r = co._r;
	_g = co._g;
	_b = co._b;
	_intensity = co._intensity;
}


#ifndef RGB_H
#define RGB_H

#include "Arduino.h"

class RGB {
	public:
		uint8_t _r;
		uint8_t _g;
		uint8_t _b;
		int _intensity;


		RGB();
		RGB(uint8_t r, uint8_t g, uint8_t b);
		uint32_t getColor();

		friend bool operator== (RGB &c1, RGB &c2);
		friend bool operator!= (RGB &c1, RGB &c2);
		void operator=(const RGB &co);
};

#endif // RGB_H

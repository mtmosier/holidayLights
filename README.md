holidayLights
=============

A simple display of holiday lights.


Purpose
=======

A simple holiday light display, it is set up with a Halloween and Christmas modes as well as timed modes, where the lights only activate during specific hours of the day.


Prerequisites
=============

At a minimum you need the Adafruit WS2801 Library.

https://github.com/adafruit/Adafruit-WS2801-Library

If you choose to use an RTC module as well, you'll want the arduino Time, Time Zone, and RTC libraries.  The RTC library below I forked to add a couple of convenience functions.  If you use the original you will need to alter this program to use the correct getHour and getMonth function formats.

https://github.com/PaulStoffregen/Time
https://github.com/JChristensen/Timezone
https://github.com/mtmosier/DS3231


Hardware
========

For the arduino, I used a generic [Arduino Pro Mini compatible microcontroller](http://www.ebay.com/itm/400762710802?_trksid=p2059210.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT), controlling a [WS2801 RGB LED Pixel Strand](https://www.adafruit.com/product/738).  An [RTC Module](http://www.ebay.com/itm/400503978923?_trksid=p2059210.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT) is also used, but is optional.  See the [schematic](_schematic.png) for more information.


Installation
============

Open the holidayLights.ino file in the Arduino IDE, change anything needed and upload to your arduino.


Further Information
===================

Pictures and description can be found at [mtmosier.com](http://mtmosier.com/80-arduino/78-window-holiday-lights).


Copyright Information
=====================

All code contained is licensed as GPLv3.

All code is Copyright 2015, Michael T. Mosier (mtmosier at gmail dot com).

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.

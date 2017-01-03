/*
FastPWMdac
For the output pins that can be used see: http://www.pjrc.com/teensy/td_libs_TimerOne.html
The library <TimerOne.h> has to be installed too.
Copyright (C) 2015  Albert van Dalen http://www.avdweb.nl
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License at http://www.gnu.org/licenses .
Version 19-12-2015
*/
#ifndef FastPWMdac_H
#define FastPWMdac_H
class FastPWMdac
{
public:
  void init(uint8_t _timer1PWMpin, uint8_t resolution);
  void analogWrite8bit(uint8_t value8bit);
  void analogWrite10bit(int value10bit);
private:
  uint8_t timer1PWMpin;
};
#endif

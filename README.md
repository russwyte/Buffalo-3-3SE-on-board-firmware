# Buffalo-3SE-on-board-firmware

On-Board Firmware for the Buffalo 3-SE DAC

This firmware depends on the watch dog timer always being on and the brownout detection being enabled (at 2.7V).

AVR - ATTiny85
Fuse bytes:
- Low:	D2
- High:	6D
- Ext:	FF

Travis should build the HEX
[![Build Status](https://travis-ci.org/russwyte/Buffalo-3SE-on-board-firmware.svg?branch=master)](https://travis-ci.org/russwyte/Buffalo-3SE-on-board-firmware)

The MIT License (MIT)

Copyright (c) 2016 Russ White

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

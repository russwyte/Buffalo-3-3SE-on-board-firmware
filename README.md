# Buffalo-3SE-on-board-firmware

On-Board Firmware for the Twisted Pear Audio - Buffalo 3/3-SE DACs

This firmware depends on the watch dog timer always being on and the brownout detection being enabled (at 2.7V).

AVR - ATTiny85
Fuse bytes:
- Low:	0xD2
- High:	0xCD
- Ext:	0xFF

[![Build Status](https://travis-ci.org/russwyte/Buffalo-3-3SE-on-board-firmware.svg?branch=master)](https://travis-ci.org/russwyte/Buffalo-3-3SE-on-board-firmware)

[Latest Release](https://github.com/russwyte/Buffalo-3-3SE-on-board-firmware/releases/latest "latest release")

The switches are connected to the Port Expander GPIO pins with weak pull-ups enabled. The other end of the switch is connected to GND - thus "Off" is open and results in logic 1 and "On" is closed and results in logic 0.

# Buffalo-3/3SE Switch Assignments
| POS | Switch 1                                | Switch 2                                                            | POS |
|-----|-----------------------------------------|---------------------------------------------------------------------|-----|
| 1   | Quantizer Bit-0                         | IIR Freq Bit-1                                                      | 8   |
| 2   | Quantizer Bit-1                         | IIR Freq Bit-0                                                      | 7   |
| 3   | 0 - True Diff <br>1 - Pseudo Diff       | 0 - SPDIF auto-detect ON <br>1 - SPDIF auto-detect OFF(*See Note 1) | 6   |
| 4   | 0 - Fast Roll-Off <br>1 - Slow Roll-Off | 0 - Stereo Mode <br>1 - Mono Mode (*See Note 2)                     | 5   |
| 5   | DPLL Bit-0                              | 0 - Normal <br>1 - No Jitter Eliminator or OSF                      | 4   |
| 6   | DPLL Bit-1                              | MODE Bit-2                                                          | 3   |
| 7   | DPLL Bit-2                              | MODE Bit-1                                                          | 2   |
| 8   | 0 - DPLL BW X 1 <br>1 - DPLL BW X 128   | MODE Bit-0                                                          | 1   |


Note 1: SPIDF auto-detection should always be disabled when playing PCM if possible otherwise at high sample rates random unlocks can happen because of false positives from the detector.

Note 2: In MONO mode the DAC with the address jumper open is the LEFT channel and has the LEFT output in normal phase and RIGHT output opposite phase, the DAC with the address jumper closed is the RIGHT channel and has the RIGHT output in normal phase and the LEFT output opposite phase.

Note 3: The B3 can be used as either an 8 channel DAC or as a stereo/mono DAC with the inputs re-mapped like B3SE. This allows the use of modules like "Sidecar" for stereo PCM/DSD and SPDIF inputs as well.

Quantizer settings:
* 0b00 == 6-bit
* 0b01 == 7-bit
* 0b10 == 8-bit
* 0b11 == 9-bit

DPLL Settings:
* 0b000 - Factory Default
* 0b001 - Lowest BW
* ...
* 0b111 - Most Bandwidth

Note: The DPLL of the ES9018 will drop itself out (freewheel) once a signal is locked using a synchronous master clock. In order to get a fast lock just set the DPLL value high. In asynchronous mode choose the lowest practical value while still maintaining a stable lock on the source.

IIR Freq:
* 0b00 == Normal (Lowest PCM ripple in band)
* 0b01 == 50Khz
* 0b10 == 60Khz
* 0b11 == 70Khz

Modes:
* 0b000 == I2S-32-bit
* 0b001 == Left Justified-32-bit
* 0b100 == Right Justified-32-bit
* 0b101 == Right Justified-24-bit
* 0b110 == Right Justified-20-bit
* 0b111 == Right Justified-16-bit

SPDIF-IN(B3 only):
* 0b00 == D1
* 0b01 == D2
* 0b10 == D3
* 0b11 == D4

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

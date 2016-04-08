// Copyright (c) 2016 Russ White
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include "USI_I2C_Master.h"
#include "Buffalo.h"

// switch states coming from the port expander
uint8_t sw1, sw2;

// desired DAC state
uint8_t state[26];

// array to hold ADC samples for volume
uint8_t volumeResults[AVG_LEN];

// use for quick sort
int cmp_chars(const void *c1, const void *c2) {
	uint8_t ch1 = *(uint8_t *) c1;
	uint8_t ch2 = *(uint8_t *) c2;
	return ch1 - ch2;
}

// sends a byte to the target address awaiting start condition
// the start must be successful to move on
void i2cSendByte(uint8_t address, uint8_t reg, uint8_t val) {
	if (i2c_start(address) == 0) {
		i2c_write(reg);
		i2c_write(val);
		i2c_stop();
	} else {
		i2cSendByte(address, reg, val);
	}
}

// receives a byte to the target address awaiting start condition
// the start must be successful to move on
// this prevents initializing while the DAC is not ready
uint8_t i2cReceiveByte(uint8_t address, uint8_t reg) {
	wdt_reset();
	uint8_t res = 0;
	if (i2c_start(address) == 0) {
		i2c_write(reg);
		i2c_stop();
		i2c_start(address + 1);
		res = i2c_read(1); // send nack
		i2c_stop();
	} else {
		_delay_ms(1);
		res = i2cReceiveByte(address, reg);
	}
	return res;
}

// read the port expander switch states
void readSwitchStates() {
	_delay_ms(2);
	sw1 = i2cReceiveByte(PE_ADDRESS, PE_GPIOA);
	_delay_ms(2);
	sw2 = i2cReceiveByte(PE_ADDRESS, PE_GPIOB);
}

// apply the volume to the attenuation registers in the state
void setVolume(uint8_t vol) {
	uint8_t v = 127 - vol;
	if (v >= 126)
		v = 255; // close to mute
	uint8_t i;
	for (i = 0; i < 8; i++) {
		state[i] = v;
	}
}

// gets the reading from the ADC and applies some over-sampling to get smoother reading.
// this is faster than averaging
uint8_t getVolume() {
	uint8_t i;
	//Remove jitter from ADC reading
	for (i = 0; i < AVG_LEN; i++) {
		sbi(ADCSRA, ADEN);
		ADCSRA |= _BV(ADSC); // set
		while (ADCSRA & _BV(ADSC))
			; // wait for it...
		cbi(ADCSRA, ADEN);
		volumeResults[i] = (ADCH >> 1);
	}
	qsort(volumeResults, AVG_LEN, sizeof(uint8_t), &cmp_chars);
	return volumeResults[AVG_LEN / 2];
}

void setRegisterBit(uint8_t reg, uint8_t b) {
	state[reg] = state[reg] | _BV(b);
}

void clearRegisterBit(uint8_t reg, uint8_t b) {
	state[reg] = state[reg] & ~_BV(b);
}

// Go through the state and check it against the DAC registers
// Apply the local state to the DAC register if it differs from the target register
// There is also some logic here to support MONO operation
void checkAndUpdate(uint8_t address) {
	if (MONO) {
		// deal with MONO mode
		if (address == DAC_ADDRESS + 2) {
			// Right mode
			setRegisterBit(17, 0);
			setRegisterBit(17, 7);
			// special case for re-map mode
#ifdef B3_MODE
			if(REMAP) {
				state[13] = 0b00010001;
			} else {
				state[13] = 0b01010101;
			}
#else
			state[13] = 0b00010001;
#endif
		} else {
			// Left mode
			setRegisterBit(17, 0);
			clearRegisterBit(17, 7);
			// special case for re-map mode
#ifdef B3_MODE
			if (REMAP) {
				state[13] = 0b00100010;
			} else {
				state[13] = 0b10101010;
			}
#else
			state[13] = 0b00100010;
#endif
		}
	} else {
		// stereo mode
		clearRegisterBit(17, 0);
		state[13] = 0;
	}
	uint8_t i;
	for (i = 0; i < 26; i++) {
		// we could just write each register always - but why... :)
		if (i2cReceiveByte(address, i) != state[i]) {
			i2cSendByte(address, i, state[i]);
		}
	}
}

// This does all the heavy lifting of configuring the state based on the switches
void configureDAC() {

	// SW1 is mapped to port A on the port expander.
	// SW2 is mapped to port B on the port expander.

	// switches 1 and 2 will for a two bit register for setting the quantizer depth
	// 00 = 6 bit
	// 01 = 7 bit
	// 10 = 8 bit
	// 11 = 9 bit;
	switch (sw1 & 0b00000011) {
	case 0b00:
		state[15] = 0b00000000;
		break;
	case 0b01:
		state[15] = 0b01010101;
		break;
	case 0b10:
		state[15] = 0b10101010;
		break;
	case 0b11:
		state[15] = 0b11111111;
		break;
	}
	// switch 3 is used to set pseudo(1) or true(0) differential because I want true as the default
	if (sw1 & _BV(2)) {
		clearRegisterBit(14, 3); // pseudo
	} else {
		setRegisterBit(14, 3); // true (default)
	}

	// switch 4 is used to fast or slow roll-off we want to the default to be fast
	if (sw1 & _BV(3)) {
		clearRegisterBit(14, 0); // slow roll off
	} else {
		setRegisterBit(14, 0); // fast roll off (default)
	}

	// Ok lets use switches 5, 6, and 7 to choose DPLL bandwidth
	// Leaving them low will be the default (do nothing).
	uint8_t dpll = ((uint8_t) sw1 >> 4) & 0b111;
	if (dpll != 0) {
		clearRegisterBit(25, 1);
		clearRegisterBit(11, 4);
		clearRegisterBit(11, 3);
		clearRegisterBit(11, 2);
		state[11] = state[11] | dpll << 2;
	} else {
		setRegisterBit(25, 1);
	}

	// Switch 8 will control DPLL BW x 128 feature.
	if (sw1 & _BV(7)) {
		setRegisterBit(25, 0); // X 128
	} else {
		clearRegisterBit(25, 0); // normal
	}

#ifdef B3_MODE
	// DIP switch 2 settings for B3

	// SPDIF selection
	switch (sw2 & 0b00000011) {
		case 0b00 :
			state[18] = 0b00000001;
			break;
		case 0b01 :
			state[18] = 0b00000010;
			break;
		case 0b10 :
			state[18] = 0b00000100;
			break;
		case 0b11 :
			state[18] = 0b00001000;
			break;
	}

	// Switch 4 disables jitter reduction and OSF.
	if (sw2 & _BV(3)) {
		// disable
		setRegisterBit(17, 6);
		clearRegisterBit(10, 2);
	} else {
		// normal-enable
		clearRegisterBit(17, 6);
		setRegisterBit(10, 2);
	}

	// switch 5 disables SPDIF auto-detect
	if (sw2 & _BV(4)) {
		clearRegisterBit(17, 3);// Don't detect
	} else {
		setRegisterBit(17, 3);// Auto-detect
	}

	// switch 6 and 7 set IIR freq
	if (sw2 & _BV(5)) {
		setRegisterBit(14, 1);
	} else {
		clearRegisterBit(14, 1);
	}
	if (sw2 & _BV(6)) {
		setRegisterBit(14, 2);
	} else {
		clearRegisterBit(14, 2);
	}

	// re-map like B3SE
	if (REMAP) {
		setRegisterBit(14, 7);
		setRegisterBit(14, 6);
		setRegisterBit(14, 5);
		setRegisterBit(14, 4);
	} else {
		clearRegisterBit(14, 7);
		clearRegisterBit(14, 6);
		clearRegisterBit(14, 5);
		clearRegisterBit(14, 4);
	}
#else
	// DIP switch 2 settings for B3 SE
	// PCM mode settings
	switch (sw2 & 0b00000111) {
	case 0b000:
		// I2S to 32 bit
		setRegisterBit(10, 7);
		setRegisterBit(10, 6);
		clearRegisterBit(10, 5);
		clearRegisterBit(10, 4);
		break;
	case 0b001:
		// LJ to 32 bit
		setRegisterBit(10, 7);
		setRegisterBit(10, 6);
		clearRegisterBit(10, 5);
		setRegisterBit(10, 4);
		break;
	case 0b100:
		// RJ 32 bit
		setRegisterBit(10, 7);
		setRegisterBit(10, 6);
		setRegisterBit(10, 5);
		clearRegisterBit(10, 4);
		break;
	case 0b101:
		// RJ 24 bit
		clearRegisterBit(10, 7);
		clearRegisterBit(10, 6);
		setRegisterBit(10, 5);
		clearRegisterBit(10, 4);
		break;
	case 0b110:
		// RJ 20 bit
		clearRegisterBit(10, 7);
		setRegisterBit(10, 6);
		setRegisterBit(10, 5);
		clearRegisterBit(10, 4);
		break;
	case 0b111:
		// RJ 16 bit
		setRegisterBit(10, 7);
		clearRegisterBit(10, 6);
		setRegisterBit(10, 5);
		clearRegisterBit(10, 4);
		break;
	}

	// Switch 4 set MONO - dealt with elsewhere

	// Switch 5 disables jitter reduction and OSF.
	if (sw2 & _BV(4)) {
		// disable
		setRegisterBit(17, 6);
		clearRegisterBit(10, 2);
	} else {
		// enable
		clearRegisterBit(17, 6);
		setRegisterBit(10, 2);
	}

	// switch 6 disables SPDIF auto-detect
	if (sw2 & _BV(5)) {
		clearRegisterBit(17, 3);	// Don't detect
	} else {
		setRegisterBit(17, 3);	// Auto-detect
	}

	// switch 7 and 8 set IIR freq
	// 0b00 == Normal
	// 0b01 == 50Khz
	// 0b10 == 60Khz
	// 0b11 == 70Khz
	if (sw2 & _BV(6)) {
		setRegisterBit(14, 1);
	} else {
		clearRegisterBit(14, 1);
	}
	if (sw2 & _BV(7)) {
		setRegisterBit(14, 2);
	} else {
		clearRegisterBit(14, 2);
	}
#endif
}

void initialize() {
	// It is critical to use the watch dog - make sure you set the fuse!
	wdt_reset();
	wdt_enable(WDTO_60MS);
	// default values (slightly tweaked) - see the ES9018 data sheet
	// 0 - 7 are the volume registers
	// just to be cautious we will set attenuation to max
	state[0] = 255;
	state[1] = 255;
	state[2] = 255;
	state[3] = 255;
	state[4] = 255;
	state[5] = 255;
	state[6] = 255;
	state[7] = 255;
	// settings
	state[8] = 0b01111111;
	state[9] = 0b00000000;
	state[10] = 0b11001110;
	state[11] = 0b10011101;
	state[12] = 0b00100000;
	state[13] = 0b00000000;
#ifdef B3_MODE
	state[14] = 0b00001001;// we always re-map for B3SE - also use normal IIR
#else
	state[14] = 0b11111001;// we always re-map for B3SE - also use normal IIR
#endif
	state[15] = 0b00000000;
	state[16] = 0b00000000;
	state[17] = 0b00011100;
	state[18] = 0b00000001;
	state[19] = 0b00000000;
	// master trim - no attenuation
	state[20] = 0xFF;
	state[21] = 0xFF;
	state[22] = 0xFF;
	state[23] = 0x7F;
	// more settings
	state[24] = 0b00110000;
	state[25] = 0b00000010;

	// ADC setup
	cbi(DDRB, PIN4);// set DDRB4 to input
	cbi(PORTB, PIN4);	// disable weak pull-up on PORTB4
	// setup ADC
	ADCSRA = 0;
	ADMUX = 0;
	sbi(ADMUX, ADLAR);
	sbi(ADMUX, MUX1);
	cbi(ADCSRA, ADEN);
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);
	// setup I2C
	i2c_init();
	_delay_ms(1);
	// configure the port expander
	i2cSendByte(PE_ADDRESS, PE_IOCONN, 0b00100000);	// we will only be sending one byte at a time
	_delay_ms(1);
	i2cSendByte(PE_ADDRESS, PE_GPPUA, 0b11111111); // enable all weak pull-ups
	i2cSendByte(PE_ADDRESS, PE_GPPUB, 0b11111111); // enable all weak pull-ups
	_delay_ms(1);
	// GET the initial state for the DAC registers
}

// Main
int main(int argc, char **argv) {
	initialize();
	// Continually check the state/ADC and apply changes to the DAC(s)
	while (1) {
		wdt_reset();
		// kick the dog
		readSwitchStates();
		setVolume(getVolume());
		configureDAC();
		checkAndUpdate(DAC_ADDRESS);
		if (MONO) {
			wdt_reset();
			// kick the dog again because we are doing this twice
			checkAndUpdate(DAC_ADDRESS + 2);
		}
		_delay_ms(10); // sleep a bit
	}
	return 0;
}

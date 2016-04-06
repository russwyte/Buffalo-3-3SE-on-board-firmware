// Copyright (c) 2016 Russ White

#ifndef BUFFALO_H_
#define BUFFALO_H_
#define DAC_ADDRESS 0x90

#define PE_ADDRESS  			0x40

#define sbi(sfr, bit) 			(_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) 			(_SFR_BYTE(sfr) &= ~_BV(bit))

#define AVG_LEN 20 // The number of times to sample ADC

#define PE_IOCONN				0x0A// at least initially
#define PE_IPOLA  				0x02
#define PE_IPOLB  				0x03
#define PE_GPPUA  				0x0C
#define PE_GPPUB  				0x0D
#define PE_GPIOA				0x12
#define PE_GPIOB				0x13
#define MONO 					sw2 & _BV(3)
#endif /* BUFFALO_H_ */

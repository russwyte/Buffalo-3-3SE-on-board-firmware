// Copyright (c) 2016 Russ White

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

#include "USI_I2C_Master.h"

#ifndef F_CPU
#define F_CPU 8000000	      // Sets up the default speed for delay.h
#endif

void i2c_init(void);
uint8_t USI_I2C_Master_Transfer(uint8_t);
uint8_t i2c_get_state_info(void);
uint8_t i2c_start(uint8_t);
uint8_t i2c_rep_start(uint8_t);
uint8_t i2c_stop(void);
uint8_t i2c_write(uint8_t);
uint8_t i2c_read(uint8_t);

union  USI_I2C_state
{
	uint8_t errorState;         // Can reuse the I2C_state for error states since it will not be needed if there is an error.
	struct
	{
		uint8_t addressMode         : 1;
		uint8_t masterWriteDataMode : 1;
		uint8_t memReadMode		    : 1;
		uint8_t unused              : 5;
	};
}   USI_I2C_state;


/*---------------------------------------------------------------
USI I2C single master initialization function
---------------------------------------------------------------*/
void i2c_init(void)
{
	PORT_USI |= (1<<PIN_USI_SDA);           // Enable pullup on SDA, to set high as released state.
	PORT_USI |= (1<<PIN_USI_SCL);           // Enable pullup on SCL, to set high as released state.
	
	DDR_USI  |= (1<<PIN_USI_SCL);           // Enable SCL as output.
	DDR_USI  |= (1<<PIN_USI_SDA);           // Enable SDA as output.
	
	USIDR    =  0xFF;                       // Preload dataregister with "released level" data.
	USICR    =  (0<<USISIE)|(0<<USIOIE)|                            // Disable Interrupts.
	(1<<USIWM1)|(0<<USIWM0)|                            // Set USI in Two-wire mode.
	(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|                // Software strobe as counter clock source
	(0<<USITC);
	USISR   =   (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Clear flags,
	(0x0<<USICNT0);                                     // and reset counter.
}

/*---------------------------------------------------------------
Use this function to get hold of the error message from the last transmission
---------------------------------------------------------------*/
uint8_t i2c_get_state_info(void)
{
	return (USI_I2C_state.errorState);                            // Return error state.
}

/*---------------------------------------------------------------
Core function for shifting data in and out from the USI.
Data to be sent has to be placed into the USIDR prior to calling
this function. Data read will be returned from the function.
This function leaves the bus in the following state:
SCL low.
SDA high [and as output].
---------------------------------------------------------------*/
uint8_t USI_I2C_Master_Transfer(uint8_t temp)
{
	USISR = temp;                                     // Set USISR according to temp.
	// Prepare clocking.
	temp  =  (0<<USISIE)|(0<<USIOIE)|                 // Interrupts disabled
	(1<<USIWM1)|(0<<USIWM0)|                 // Set USI in Two-wire mode.
	(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|     // Software clock strobe as source.
	(1<<USITC);                              // Toggle Clock Port.
	do
	{
		_delay_us(T2_I2C);
		USICR = temp;							// Generate positive SCL edge.
		while ( !(PIN_USI & (1<<PIN_USI_SCL)) );	// Wait for SCL to go high.
		_delay_us(T4_I2C);
		USICR = temp;							// Generate negative SCL edge.
	} while ( !(USISR & (1<<USIOIF)) );			// Check for transfer complete.
	
	_delay_us(T2_I2C);
	temp  = USIDR;                           // Read out data.
	
	USIDR = 0xFF;                            // Release SDA.
	DDR_USI |= (1<<PIN_USI_SDA);             // Enable SDA as output.

	return temp;                             // Return the data from the USIDR
}

/*---------------------------------------------------------------
Function for generating a TWI Start Condition.
Start condition is SDA going low while SCL is high.
---------------------------------------------------------------*/
uint8_t i2c_start(uint8_t addr)
{
	/* Release SCL to ensure that (repeated) Start can be performed */
	PORT_USI |= (1<<PIN_USI_SCL);                     // Release SCL.
	while ( !(PORT_USI & (1<<PIN_USI_SCL)) );          // Verify that SCL becomes high.
	_delay_us(T2_I2C);

	/* Generate Start Condition */
	PORT_USI &= ~(1<<PIN_USI_SDA);                    // Force SDA LOW.  <-- this is the I2C Start!
	_delay_us(T4_I2C);
	PORT_USI &= ~(1<<PIN_USI_SCL);                    // Pull SCL LOW.
	_delay_us(T4_I2C);	//delay in here - see http://8515.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=659402
	PORT_USI |= (1<<PIN_USI_SDA);                     // Release SDA.

	#ifdef SIGNAL_VERIFY
	if ( !(USISR & (1<<USISIF)) )
	{
		USI_I2C_state.errorState = USI_I2C_MISSING_START_CON;
		return (1);
	}
	#endif
	
	if (i2c_write(addr))
	{
		return (0);
	}
	else
	{
		USI_I2C_state.errorState = USI_I2C_NO_ACK_ON_ADDRESS;
		return (1);
	}
}

/*---------------------------------------------------------------
Issues a repeated start condition and sends address and transfer
direction.
A repeated start is just another start: SDA low while SCL high.
---------------------------------------------------------------*/
uint8_t i2c_rep_start(uint8_t addr)
{
	uint8_t tmp;
	
	if (i2c_start(addr))
	{
		tmp = 1;
	}
	else
	{
		tmp = 0;
	}
	return tmp;
}

/*---------------------------------------------------------------
Issues a start condition and sends address and transfer direction.
If device is busy, use ack polling to wait until device is ready
---------------------------------------------------------------*/
uint8_t i2c_start_wait(uint8_t addr)
{
	if (i2c_start(addr) == 1) {
		i2c_stop();
		_delay_ms(10);
		i2c_start_wait(addr); 
	}
	return(0);
}

/*---------------------------------------------------------------
Function for generating a TWI Stop Condition. Used to release
the TWI bus.
A stop condition is SDA going high while SCL is high.
---------------------------------------------------------------*/
uint8_t i2c_stop(void)
{
	PORT_USI &= ~(1<<PIN_USI_SDA);           // Pull SDA low.
	PORT_USI |= (1<<PIN_USI_SCL);            // Release SCL.
	while ( !(PIN_USI & (1<<PIN_USI_SCL)) );  // Wait for SCL to go high.
	_delay_us(T4_I2C);
	PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
	_delay_us(T2_I2C);
	
	#ifdef SIGNAL_VERIFY
	if ( !(USISR & (1<<USIPF)) )
	{
		USI_I2C_state.errorState = USI_I2C_MISSING_STOP_CON;
		return (1);
	}
	#endif

	return (0);
}


/*---------------------------------------------------------------
Function for writing one byte to a slave device.
Returns ack/nack state.
0 = ack
1 = nack
---------------------------------------------------------------*/
uint8_t i2c_write(uint8_t data)
{
	uint8_t temp;
	uint8_t const tempUSISR_8bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0);	// Prepare register value to: Clear flags, and
	// set USI to shift 8 bits i.e. count 16 clock edges.
	uint8_t const tempUSISR_1bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0xE<<USICNT0);	// Prepare register value to: Clear flags, and
	// set USI to shift 1 bit i.e. count 2 clock edges.
	
	DDR_USI  |= (1<<PIN_USI_SDA);	// Enable SDA as output.
	PORT_USI &= ~(1<<PIN_USI_SCL);	// Pull SCL LOW.
	USIDR = data;
	
	USI_I2C_Master_Transfer(tempUSISR_8bit);	//Send the data.
	
	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);	// Enable SDA as input.
	temp = USI_I2C_Master_Transfer(tempUSISR_1bit);	//Get (n)ack.
	//PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
	
	if (temp)
	{
		USI_I2C_state.errorState = USI_I2C_NO_ACK_ON_DATA;
	}
	return temp;	//Should contain (N)ACK.
}

/*---------------------------------------------------------------
Function for reading one byte from a slave device.
Takes a parameter ack_b where:
0 = ack
1 = nack
Returns the data.
---------------------------------------------------------------*/
uint8_t i2c_read(uint8_t ack_b)
{
	uint8_t temp;
	uint8_t const tempUSISR_8bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0);	// Prepare register value to: Clear flags, and
	// set USI to shift 8 bits i.e. count 16 clock edges.
	uint8_t const tempUSISR_1bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0xE<<USICNT0);	// Prepare register value to: Clear flags, and
	// set USI to shift 1 bit i.e. count 2 clock edges.
	
	DDR_USI  &= ~(1<<PIN_USI_SDA);	// Enable SDA as input.
	PORT_USI &= ~(1<<PIN_USI_SCL);	// Pull SCL LOW.
	USIDR = 0;
	
	temp = USI_I2C_Master_Transfer(tempUSISR_8bit);	//Get the data.
	
	/* Clock and verify (N)ACK to slave */
	DDR_USI  |= (1<<PIN_USI_SDA);	// Enable SDA as output.
	if (ack_b == 0)
	{
		USIDR = 0x00;
	}
	else
	{
		USIDR = 0xff;
	}
	USI_I2C_Master_Transfer(tempUSISR_1bit);	//Send (n)ack.
	//PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
	
	return temp;	//Should contain the data.
}

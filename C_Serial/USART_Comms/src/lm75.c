#include "i2cmaster.h"
#include "lm75.h"

void lm75_init()
{
	// Set sensor configuration
	i2c_start_wait(LM75_ADR + I2C_WRITE);
	// In the pointer register, request to write the configuration register
	i2c_write(0x01); 
	// Write the configuration register to normal mode and default params
	i2c_write(0x00);
	i2c_stop();
}

double get_temperature() {
	
	float temperature; // will hold the measured ambient temperature in deg. Celsius
	unsigned int temp_data; // intermediate value
	unsigned char low_temp_byte;
	unsigned char high_temp_byte;

	// In the pointer register, request to read from Temp register
	i2c_start_wait(LM75_ADR + I2C_WRITE);
	i2c_write(0x00); 
	
	// The LM75a will respond with 2 bytes
	// Temperature returned as follows:
	// High byte: | D10 | D09 | D08 | D07 | D06 | D05 | D04 | D03 | 
	// Low byte:  | D02 | D01 | D00 | xxx | xxx | xxx | xxx | xxx |
	// where D10 is the sign bit and D09..D00 is the value of the temp * 0.125
	// if temp is negative, then temp is stored as 2's complement
	i2c_rep_start(LM75_ADR + I2C_READ);
	high_temp_byte = i2c_readAck();
	low_temp_byte = i2c_readNak();
	i2c_stop();
	
	if ((high_temp_byte & 0x80)==0) { // temp. >= 0
		temp_data = (((high_temp_byte & 0x7F) << 3) | (low_temp_byte >> 5)) ;
		temperature = temp_data * 0.125;
	}
	else { // temp. < 0
		temp_data = (((high_temp_byte & 0x7F) << 3) | (low_temp_byte >> 5)) ;
		temp_data = ~temp_data + 1;
		temperature = 0.00 - temp_data;
		
	}
	return temperature;
}

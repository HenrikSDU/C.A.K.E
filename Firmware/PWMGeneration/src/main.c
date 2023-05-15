/* Macros for GPIO pins */
#define FORWARD_PIN PORTD2
#define BACKWARD_PIN PORTD3

//The SDU CAKE program
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "lcd.h"
#include "usart.h"
#include "i2cmaster.h"
#include "lm75.h"


void PWM_T0_init(void){
    DDRD |=0x60;
//PWM Setup, see page 84 and following in datasheet for register description
TCCR0A |= ((1<<COM0A1)|(1<<WGM01)|(1<<WGM00)); //configuring fast PWM, clear OC0A pin on compare match set it at BOTTOM
TCCR0B |= ((1<<CS02)|(1<<CS00)); //setting prescaler to 1024
}
void PWM_T0_set(unsigned char PWM_val){

    OCR0A = PWM_val;
    
}

void PWM_T0_direction_change(int direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTD &= ~(1 << BACKWARD_PIN);
        PORTD |= (1 << FORWARD_PIN);
    }
    if(direction == 0) {
        PORTD &= ~(1 << FORWARD_PIN);
        PORTD |= (1 << BACKWARD_PIN);
    }
}


int main(void) { 

  
    
    i2c_init();//initialize I2C communication
    
    //LCD_init();//initialize the LCD
    lm75_init();//initialize the temperature sensor

    //configuration of the IO pins

    DDRD = 0xFF;
    PORTD= 0x00;
    DDRC = 0xF0;
    PORTC = 0x3F;
    DDRB |= (1<<PB5);

    PWM_T0_init();
    PWM_T0_set(255);

    PORTD |= (1<<PD2);
    PORTD |= (1<<PD6);
    PORTB |= (1<<PB5);
    
    uint8_t direction=0;

    while(1){

        PWM_T0_direction_change(0b00000001 & direction);
        _delay_ms(1000);
        direction++;

    }
    

       

    
        
    return 0;
}



/* Macros for GPIO pins */
#define FORWARD_PIN PORTD2
#define BACKWARD_PIN PORTD3

#define BUTTON0 PD2
#define BUTTON1 PD3
#define BUTTON2 PD7
#define BUTTON3 PC0
#define BUTTON4 PC1
#define BUTTON5 PC2
#define BUTTON6 PC3



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

ISR(PCINT1_vect){

    if((PINC & (1 << BUTTON4)) == 0)
        PORTB ^= (1<<PB5);
        //printf("hey");

}

void PWM_T0A_init(void){

    DDRD |= (1<<PD6); //configuring OC0A pin to be output
    //PWM Setup, see page 84 and following in datasheet for register description
    TCCR0A |= ((1<<COM0A1)|(1<<WGM01)|(1<<WGM00)); //configuring fast PWM, clear OC0A pin on compare match set it at BOTTOM
    TCCR0B |= ((1<<CS02)|(1<<CS00)); //setting prescaler to 1024

}

void PWM_T0B_init(void){

    DDRD |= (1<<PD5); //configuring OC0B pin to be output
    //PWM Setup, see page 84 and following in datasheet for register description
    TCCR0A |= ((1<<COM0B1)|(1<<WGM01)|(1<<WGM00)); //configuring fast PWM, clear OC0A pin on compare match set it at BOTTOM
    TCCR0B |= ((1<<CS02)|(1<<CS00)); //setting prescaler to 1024

}

void PWM_T0A_set(unsigned char PWM_val){

    OCR0A = PWM_val;
    
}

void PWM_T0B_set(unsigned char PWM_val){

    OCR0B = PWM_val;
    
}


void PWM_T0A_direction_change(int direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTD &= ~(1 << BACKWARD_PIN);
        PORTD |= (1 << FORWARD_PIN);
    }
    if(direction == 0) {
        PORTD &= ~(1 << FORWARD_PIN);
        PORTD |= (1 << BACKWARD_PIN);
    }
}

void button_init(void){


    //configuring IOPins
    DDRC &= ~((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //configuring them as input
    PORTC |= ((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //enabling Pull-Ups
    DDRD &= ~((1 << BUTTON0) | (1 << BUTTON1));
    PORTD |= (1 << BUTTON0) | (1 << BUTTON1);


    //initializing the external interrupts - see page 54
    EICRA &= ~((1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10)); //when 0 0 any logical change generates interrupt request
    
    //enabling interrupts for both INT1 and INT0
    EIMSK |= ((1<<INT1)|(1<<INT0));

    //initializing the PinChange Interrupts
    PCICR |= ((1<<PCIE2)|(1<<PCIE1)); //enabeling pin interrupts of pin group 1 and 2 
    PCMSK2 |= (1<<BUTTON2);
    PCMSK1 |= ((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //subscribing to changes on PCINT9
    sei();
    


}


int main(void) { 

  
    
    i2c_init();//initialize I2C communication
    
    LCD_init();//initialize the LCD
    printf("LCDinitSUCCESS");

    
    //configuration of the IO pins

    DDRC |= 0x30; //for I2C
    PORTC |= 0x30;
    DDRB |= (1<<PB5);

    //PWM_T0A_init();
    //PWM_T0A_set(255);

    //PORTD |= (1<<PD2);
    //PORTD |= (1<<PD6);
    //uint8_t direction=0;

    button_init();

    while(1){

        //PWM_T0A_direction_change(0b00000001 & direction);
        //_delay_ms(1000);
        //direction++;

    }
    

       

    
        
    return 0;
}



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

#define DIRECTION_F_B PB1
#define DIRECTION_B_B PB2
#define DIRECTION_F_A PB3
#define DIRECTION_B_A PB4



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



void PWM_T0A_init(void){
    // PWM signal output pin setup
    DDRD |= (1<<PD6); //configuring OC0A pin to be output    
    
    //Direction toggle pins setup, default direction = forward
    DDRB |= ((1<<DIRECTION_B_A)|(1<<DIRECTION_F_A));
    PORTB |= (1<<DIRECTION_F_A);
    PORTB &= ~(1<<DIRECTION_B_A);

    //PWM Setup, see page 84 and following in datasheet for register description
    TCCR0A |= ((1<<COM0A1)|(1<<WGM01)|(1<<WGM00)); //configuring fast PWM, clear OC0A pin on compare match set it at BOTTOM
    TCCR0B |= ((1<<CS02)|(1<<CS00)); //setting prescaler to 1024
}

void PWM_T0B_init(void){
    // PWM signal output pin setup
    DDRD |= (1<<PD5); //configuring OC0B pin to be output
    
    // Direction toggle pins setup, default direction = forward
    DDRB |= (1 << DIRECTION_F_B) | (1 << DIRECTION_B_B);
    PORTB |= (1 << DIRECTION_F_B);
    PORTB &= ~(1 << DIRECTION_B_B);
    
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
        PORTB &= ~(1 << DIRECTION_B_A);
        PORTB |= (1 << DIRECTION_F_A);
    }
    if(direction == 0) {
        PORTB &= ~(1 << DIRECTION_F_A);
        PORTB |= (1 << DIRECTION_B_A);
    }
}

void PWM_T0B_direction_change(int direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTB &= ~(1 << DIRECTION_B_B);
        PORTB |= (1 << DIRECTION_F_B);
    }
    if(direction == 0) {
        PORTB &= ~(1 << DIRECTION_F_B);
        PORTB |= (1 << DIRECTION_B_B);
    }
}

void button_init(void){


    //configuring IOPins
    DDRC &= ~((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //configuring them as input
    PORTC |= ((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //enabling Pull-Ups
    DDRD &= ~((1 << BUTTON0) | (1 << BUTTON1) | (1 << BUTTON2));
    PORTD |= ((1 << BUTTON0) | (1 << BUTTON1) | (1 << BUTTON2));



    //initializing the external interrupts - see page 54
    EICRA &= ~((1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10)); //when 0 0 any logical change generates interrupt request
    
    //enabling interrupts for both INT1 and INT0
    EIMSK |= ((1<<INT1)|(1<<INT0));

    //initializing the PinChange Interrupts
    PCICR |= ((1<<PCIE2)/*|(1<<PCIE1)*/); //enabeling pin interrupts of pin group 1 and 2 
    PCMSK2 |= (1<<BUTTON2);
    PCMSK1 |= ((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //subscribing to changes on PCINT9
    sei();
    


}

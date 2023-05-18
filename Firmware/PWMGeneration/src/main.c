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

ISR(PCINT1_vect){

    // Button pressed
    if((PINC & (1 << BUTTON3)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON4)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON5)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON6)) == 0){
        PORTB |= (1 << PB5);
    }

    // Button unpressed
    if(!(PINC & (1 << BUTTON3)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON4)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON5)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON6)) == 0){
        PORTB &= ~(1 << PB5);
    }

}
ISR(PCINT2_vect){

    if((PIND & (1 << BUTTON2)) == 0) {
        PORTB ^= (1<<PB5);
    }
   

}

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

// PWM and linear actuator speed graph needed to tune this function properly
void PWM_control(uint8_t base_PWM, uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2) {
    int x_mod;
    if((x2 - x1) >= 0) {
        PWM_T0A_direction_change(1); // Setting x direction to forwards
        x_mod = 1; // Setting x_mod to 1 to multiply the slope by
    }
    else if((x2 - x1) < 0) {
        PWM_T0A_direction_change(0); // Setting x direction to backwards
        x_mod = -1; // Setting x_mod to -1 to multiply the slope by
    }

    if((y2 - y1) >= 0) {
        PWM_T0B_direction_change(1);
    }
    else if((y2 - y1) < 0) {
        PWM_T0B_direction_change(0);
    }

    float slope = x_mod * ((y2 - y1) / (x2 - x1));

    // First try at logic controlling overflows and underflows
    if(slope > 1) {
        while((slope * (float)base_PWM) < 255) { // The idea here is to find the maximum value of base_PWM that will not overflow the OCR0A register, the limit can be decreased as needed
            base_PWM--;
        }
        PWM_T0A_set(base_PWM);
        PWM_T0B_set((int)(slope * (float)base_PWM));
    }
    else if(slope < 1) {
        while(!((slope * (float)base_PWM) > 20)) { // The idea here is to find the minimum value of base_PWM that will not be too small to to make the motors run, while roughly achieving the target speed
            base_PWM++;
        }
        PWM_T0A_set(base_PWM);
        PWM_T0B_set((int)(slope * (float)base_PWM));
    }
    else if(slope == 1) { // Setting PWM so the motors run at roughly desired speed / sqrt(2)
        base_PWM = 127; // Some predefined value to roughly get the desired speed
        PWM_T0A_set(base_PWM);
        PWM_T0B_set(base_PWM);
    }
}


int main(void){ 
  
    i2c_init(); //  Initialization of the I2C communication    
    //LCD_init(); // Initialization of the LCD display
    //printf("LCDinitSUCCESS"); // Proof that the LCD display is working

    // Configuration of the IO pins
    DDRC |= 0x30; //for I2C
    PORTC |= 0x30;
    DDRB |= (1 << PB5); // Onboard LED

        button_init();
        //LCD_init();
        //LCD_set_cursor(0,0);


        // Initialization for IOBoard - speed measurement
        
        DDRB |= (1<<PB5);

        PWM_T0A_init();
        char desired_PWM = 0;

        ////////////////////////
        cli();
        ///////////////////////

        printf("B3:setB4:incB5:decB6:dich");
        char direction;
        while(1){
    // Button pressed
    if((PINC & (1 << BUTTON3)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON4)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON5)) == 0) {
        PORTB |= (1 << PB5);
    }
    if((PINC & (1 << BUTTON6)) == 0){
        PORTB |= (1 << PB5);
    }

    // Button unpressed
    if(!(PINC & (1 << BUTTON3)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON4)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON5)) == 0) {
        PORTB &= ~(1 << PB5);
    }
    if(!(PINC & (1 << BUTTON6)) == 0){
        PORTB &= ~(1 << PB5);
    }

        /*
            if((PINC & (1 << BUTTON3)) == 0) {
                PWM_T0A_set(desired_PWM);
                //PORTB ^= (1<<PB5);
                
                
                _delay_ms(200);
            }
            if((PINC & (1 << BUTTON4)) == 0) {
                desired_PWM += 10;
                //LCD_set_cursor(0,2);
                //printf("desPWM: %u ", desired_PWM);
                _delay_ms(200);
            }
            if((PINC & (1 << BUTTON5)) == 0) {

                desired_PWM -= 10;
                //LCD_set_cursor(0,2);
                printf("desPWM: %u ", desired_PWM);
                _delay_ms(200);
            }
            if((PINC & (1 << BUTTON6)) == 0) {
                direction = 0b00000001 & direction;
                PWM_T0A_direction_change(direction);
                //LCD_set_cursor(0,3);
                //printf("dir:%d",direction);
                direction++;
                _delay_ms(2000);
            }
        */

        }
        
    return 0;
}

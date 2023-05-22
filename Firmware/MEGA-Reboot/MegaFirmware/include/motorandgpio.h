/* Macros for GPIO pins */
#define BUTTON0 PK0
#define BUTTON1 PK1
#define BUTTON2 PK2
#define BUTTON3 PK3
#define BUTTON4 PK4
#define BUTTON5 PK5
#define BUTTON6 PK6
#define BUTTON7 PK7

#define DIRECTION_F_C PF0
#define DIRECTION_B_C PF1
#define DIRECTION_F_B PF2
#define DIRECTION_B_B PF3
#define DIRECTION_F_A PF4
#define DIRECTION_B_A PF5

/* Very fancy custom macro for easy debugging command */
#define TOGGLE_ONBOARD_LED DDRB |= 0b10000000; PORTB ^= (1 << PORTB7);

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



void PWM_T4AB_init(void) { //function to initialize the PWM for the motors moving the table

    // PWM Setup, see page 145 and following in datasheet for register description
    TCCR4A |= ((1 << COM4A1) | (1 << COM4B1) | (0 << WGM41) | (1 << WGM40)); // PWM output on 3 pins, OC4A, OC4B, OC4C
    TCCR4B |= ((0 << WGM42) | (0 << WGM43) | (1 << CS42) | (1 << CS40)); // prescaler 1024, set BOTTOM (meaning timer to be 0) and clear on compare match
    
    // PWM signal output pin setup
    DDRH |= ((1 << PH4) | (1 << PH3)); //configuring OC4 A/B/C pins to be output    
    PORTH &= ~((1 << PH4) | (1 << PH3));

    // Direction toggle pins setup, default direction = forward
    DDRF |= ((1 << DIRECTION_B_A) | (1 << DIRECTION_F_A) | (1 <<  DIRECTION_B_B) | (1 << DIRECTION_F_B));
    PORTF |= ((1 << DIRECTION_F_A) | (1 << DIRECTION_F_B));
    PORTF &= ~((1 << DIRECTION_B_A) | (1 << DIRECTION_B_B));

    //zero the PWM initially
    OCR4A = 0;
    OCR4B = 0;

    PRR1 &= ~(1 << PRTIM4);
}

void PWM_T4C_init(void) { // Function to initialize the PWM for the motor controlling the extruder
    
    TCCR4A |= (1 << COM4C1 | (1 << WGM41) | (1 << WGM40));
    TCCR4B |= ((1 << WGM42) | (1 << WGM43) | (1 << CS40));

    // PWM signal output pin setup
    DDRH |= (1 << PH5);
    PORTH &= ~((1 << PH5));
    
    // Direction toggle pins setup, default direction = forward
    DDRF |= ((1 << DIRECTION_F_C) | (1 << DIRECTION_B_C));
    PORTF |= (1 << DIRECTION_F_C);
    PORTF &= ~(1 << DIRECTION_B_C);
    
    // Zero the PWM initially
    OCR4C = 0;
}

void PWM_T4A_set(unsigned char PWM_val){
    OCR4A = PWM_val;
}

void PWM_T4B_set(unsigned char PWM_val){
    OCR4B = PWM_val;
}
void PWM_T4C_set(int PWM_val){
    OCR4C = PWM_val;
}

void PWM_T4A_direction_change(int direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTF &= ~(1 << DIRECTION_B_A);
        PORTF |= (1 << DIRECTION_F_A);
    }
    if(direction == 0) {
        PORTF &= ~(1 << DIRECTION_F_A);
        PORTF |= (1 << DIRECTION_B_A);
    }
}

void PWM_T4B_direction_change(char direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTF &= ~(1 << DIRECTION_B_B);
        PORTF |= (1 << DIRECTION_F_B);
    }
    if(direction == 0) {
        PORTF &= ~(1 << DIRECTION_F_B);
        PORTF |= (1 << DIRECTION_B_B);
    }
}

void PWM_T4C_direction_change(int direction) {
    if(direction == 1) {
        PORTF &= ~(1 << DIRECTION_B_C);
        PORTF |= (1 << DIRECTION_F_C);
    }
    if(direction == 0) {
        PORTF &= ~(1 << DIRECTION_F_C);
        PORTF |= (1 << DIRECTION_B_C);
    }
}

void button_init(void){
    // Mega buttons
    DDRK &= ~((1 << BUTTON0) | (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3) | (1 << BUTTON4) | (1 << BUTTON5) | (1 << BUTTON6) | (1 << BUTTON7));
    PORTK |= ((1 << BUTTON0) | (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3) | (1 << BUTTON4) | (1 << BUTTON5) | (1 << BUTTON6) | (1 << BUTTON7));
    
    PCICR |= (1 << PCIE2); // Pin change interrupt control register, enabled the bit where out buttons are
    PCMSK2 |= (1 << BUTTON7); // Enabling the interrupt for BUTTON7, so only when that is pressed will the interrupt be executed

    /*
    //initializing the external interrupts - see page 54
    EICRA &= ~((1 << ISC01) | (1 << ISC00) | (1 << ISC11) | (1 << ISC10)); //when 0 0 any logical change generates interrupt request
    
    //enabling interrupts for both INT1 and INT0
    EIMSK |= ((1<<INT1)|(1<<INT0));

    //initializing the PinChange Interrupts
    PCICR |= ((1<<PCIE2)|(1<<PCIE1)); //enabeling pin interrupts of pin group 1 and 2 
    PCMSK2 |= (1<<BUTTON2);
    PCMSK1 |= ((1<<BUTTON6)|(1<<BUTTON5)|(1<<BUTTON4)|(1<<BUTTON3)); //subscribing to changes on PCINT9
    sei();
    */
    


}

void PWM_control(unsigned char base_PWM, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {
    int x_mod;
    int y_mod;

    //Sim variables
    unsigned char x_dir[1];
    unsigned char y_dir[1];
    unsigned char x_speed = 0;
    unsigned char y_speed = 0;
    float dx = (float)x2 - (float)x1; // Unsigned chars are not good for division
    float dy = (float)y2 - (float)y1;

    float slope = 0.0;

    if(dx > 0) {
        PWM_T4A_direction_change(1); // Forward
        x_dir[0] = 'F'; // Forward
        x_mod = 1; // Setting x_mod to 1 to multiply the slope by
    }
    else if(dx < 0) {
        PWM_T4A_direction_change(0); // Backward
        x_dir[0] = 'B'; // Backward
        x_mod = -1; // Setting x_mod to -1 to multiply the slope by
    }
    else if(dx == 0) {
        x_dir[0] = 'S'; // Stop
        x_mod = 0; // Setting x_mod to 0 to multiply the slope by
    }

    if(dy > 0) {
        PWM_T4B_direction_change(1); // Forward
        y_dir[0] = 'F';
        y_mod = 1;
    }
    else if(dy < 0) {
        PWM_T4B_direction_change(0); // Backward
        y_dir[0] = 'B';
        y_mod = -1;
    }
    else if(dy == 0) {
        y_dir[0] = 'S';
        y_mod = 0;
    }

    
    if(x_dir[0] == 'S') {
        PWM_T4A_set(0);
        x_speed = 0;
    }
    if(y_dir[0] == 'S') {
        PWM_T4B_set(0);
        y_speed = 0;
    }
    if(x_mod != 0) {
        slope = x_mod * y_mod * (dy / dx); // Had problems with unsigned chars being divided so I used floats
    }

    // Second try at logic controlling overflows and underflows
    if(slope > 1 && x_mod != 0 && y_mod != 0) {
        if((slope * (float)base_PWM) > 255.0) { // The idea here is to find the maximum value of base_PWM that will not overflow the OCR0A register, the limit can be decreased as needed
            while((slope * (float)base_PWM) > 255.0) {
                base_PWM--;
            }
        }
        x_speed = base_PWM;
        y_speed = (int)(slope * (float)base_PWM);
    }
    else if(slope < 1 && x_mod != 0 && y_mod != 0) {
        if((slope * (float)base_PWM) > 50.0) { // The idea here is to find the minimum value of base_PWM that will not be too small to to make the motors run, while roughly achieving the target speed
            while((slope * (float)base_PWM) > 50.0) {
                base_PWM++;
            }
        }
        x_speed = base_PWM;
        y_speed = (int)(slope * (float)base_PWM);
    }
    else if(slope == 1.0 && x_mod != 0 && y_mod != 0) { // Setting PWM so the motors run at roughly desired speed / sqrt(2)
        base_PWM = 100; // Some predefined value to roughly get the desired speed
        x_speed = base_PWM;
        y_speed = base_PWM;
    }

    // Setting the PWM
    printf("\nX-Speed: %d", x_speed);
    printf("\nY-Speed: %d", y_speed);
    printf("\nSlope: %f\n", slope);

    PWM_T4A_set(x_speed);
    PWM_T4B_set(y_speed);
}

void alternative_PWM_control_init(void) {

    // Initialize input capture mode
    TCCR1B |= ((1<<ICNC1)|(1<<ICES1)); //input noise cancel & selecting rising edge for input capture
    TCCR1B |= ((1<<CS12)|(1<<CS10)); //setting prescaler of 1024

    // Enable Input Capture Interrupt and the Timer 1 Overflow interrupt
    TIMSK1 |= ((1 << ICIE1) | (1 << TOIE1));

}

void alternative_PWM_control(unsigned char base_PWM, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {





}

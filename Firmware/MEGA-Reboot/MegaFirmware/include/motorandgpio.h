// The SDU CAKE program

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


#define ACTEXTENSIONPERROT (double) 0.25 // cm
#define TICKDISTANCE (double) 0.008333333333333333

/* Very fancy custom macro for easy debugging command */
#define TOGGLE_ONBOARD_LED DDRB |= 0b10000000; PORTB ^= (1 << PORTB7);

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <math.h>

#include "lcd.h"
#include "usart.h"
#include "i2cmaster.h"
#include "lm75.h"

// External variables (they can be accessed by the file this header is included in, the word extern does this)
extern volatile double axisspeed_motor_A;
extern volatile double current_x_distance;
extern volatile double axisspeed_motor_B;
extern volatile double current_y_distance;

extern volatile unsigned char x_pos_current = 0; // Used for tracking current position of the linear actuator
extern volatile unsigned char y_pos_current = 0;
extern volatile unsigned char x_pos_next = 0; // Used for comparison between current position and the next position
extern volatile unsigned char y_pos_next = 0;

void PWM_T3AB_init(void) { //function to initialize the PWM for the motors moving the table

    // PWM Setup, see page 145 and following in datasheet for register description
    TCCR3A |= ((1 << COM3A1) | (1 << COM3B1) | (0 << WGM31) | (1 << WGM30)); // PWM output on 3 pins, OC4A, OC4B, OC4C
    TCCR3B |= ((0 << WGM32) | (0 << WGM33) | (1 << CS32) | (1 << CS30)); // prescaler 1024, set BOTTOM (meaning timer to be 0) and clear on compare match
    
    // PWM signal output pin setup
    DDRE |= ((1 << PE4) | (1 << PE3)); //configuring OC4 A/B/C pins to be output    
    PORTE &= ~((1 << PE4) | (1 << PE3));

    // Direction toggle pins setup, default direction = forward
    DDRF |= ((1 << DIRECTION_B_A) | (1 << DIRECTION_F_A) | (1 << DIRECTION_B_B) | (1 << DIRECTION_F_B));
    PORTF |= ((1 << DIRECTION_F_A) | (1 << DIRECTION_F_B));
    PORTF &= ~((1 << DIRECTION_B_A) | (1 << DIRECTION_B_B));

    //zero the PWM initially
    OCR3A = 0;
    OCR3B = 0;

    PRR1 &= ~(1 << PRTIM4);
}

void PWM_T3C_init(void) { // Function to initialize the PWM for the motor controlling the extruder
    
    TCCR4A |= (1 << COM4C1 | (1 << WGM41) | (1 << WGM40));
    TCCR4B |= ((1 << WGM42) | (1 << WGM43) | (1 << CS40));

    // PWM signal output pin setup
    DDRE |= (1 << PE5);
    PORTE &= ~((1 << PE5));
    
    // Direction toggle pins setup, default direction = forward
    DDRF |= ((1 << DIRECTION_F_C) | (1 << DIRECTION_B_C));
    PORTF |= (1 << DIRECTION_F_C);
    PORTF &= ~(1 << DIRECTION_B_C);
    
    // Zero the PWM initially
    OCR3C = 0;
}

void PWM_T3A_set(unsigned char PWM_val){
    OCR3A = PWM_val;
}

void PWM_T3B_set(unsigned char PWM_val){
    OCR3B = PWM_val;
}
void PWM_T3C_set(int PWM_val){
    OCR3C = PWM_val;
}

void PWM_T3A_direction_change(int direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTF &= ~(1 << DIRECTION_B_A);
        PORTF |= (1 << DIRECTION_F_A);
    }
    if(direction == 0) {
        PORTF &= ~(1 << DIRECTION_F_A);
        PORTF |= (1 << DIRECTION_B_A);
    }
}

void PWM_T3B_direction_change(char direction) { // direction = 1 => forwards, direction = 0 => backwards
    if(direction == 1) {
        PORTF &= ~(1 << DIRECTION_B_B);
        PORTF |= (1 << DIRECTION_F_B);
    }
    if(direction == 0) {
        PORTF &= ~(1 << DIRECTION_F_B);
        PORTF |= (1 << DIRECTION_B_B);
    }
}

void PWM_T3C_direction_change(int direction) {
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

float abs_value(float x) {
    if(x < 0) {
        return -x;
    } else {
        return x;
    }
}

void PWM_control_ext_int_init() {
    EICRA |= ((1 << ISC11) | (1 << ISC10) | (1 << ISC01) | (1 << ISC00));
    EIMSK |= ((1 << INT1) | (1 << INT0));
    sei();
}

void PWM_control(unsigned char base_PWM, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {
    unsigned char x_dir; // X direction, indicated by F, B or S
    unsigned char y_dir;
    unsigned char x_speed = 0; // Self-explanatory
    unsigned char y_speed = 0;
    float dx = (float)x2 - (float)x1; // Unsigned chars are not good for division, hence the conversion to float
    float dy = (float)y2 - (float)y1;
    float slope = 0.0; // Slope of the vector between the two points

    if(dx > 0) {
        PWM_T3A_direction_change(1); // Forward
        x_dir = 'F'; // Forward
    } else if(dx < 0) {
        PWM_T3A_direction_change(0); // Backward
        x_dir = 'B'; // Backward
    } else if(dx == 0) {
        x_dir = 'S'; // Stop
    }

    if(dy > 0) {
        PWM_T3B_direction_change(1); // Forward
        y_dir = 'F';
    } else if(dy < 0) {
        PWM_T3B_direction_change(0); // Backward
        y_dir = 'B';
    } else if(dy == 0) {
        y_dir = 'S';
    }

    dx = abs_value(dx); // Taking absolute value of dx and dy bc the sign is handled out by the direction change
    dy = abs_value(dy);

    if(x_dir == 'S' || y_dir == 'S') {
        if(x_dir == 'S') {
            PWM_T3A_set(0);
        }
        else if(x_dir != 'S') {
            PWM_T3A_set(base_PWM);
        }
        if(y_dir == 'S') {
            PWM_T3B_set(0);
        }
        else if(y_dir != 'S') {
            PWM_T3B_set(base_PWM);
        }
    } else {
        slope = dy / dx; // Had problems with unsigned chars being divided so I used floats

        if(slope < 1) {
            while((slope * base_PWM) < 60) {
                base_PWM++;
            }
            x_speed = base_PWM;
            y_speed = (int)(slope * (float)base_PWM);

        } else if(slope > 1) {
            while((slope * base_PWM) > 255) {
                base_PWM--;
            }
            x_speed = base_PWM;
            y_speed = (int)(slope * (float)base_PWM);

        } else if(slope == 1) {
            base_PWM = 100;
            x_speed = base_PWM;
            y_speed = base_PWM;
        }
        PWM_T3A_set(x_speed);
        PWM_T3B_set(y_speed);
    }
    // Debug prints
    /*
    printf("\ndx: %d - %d = %d (%f), %d - %d = %d (%f)\n", x2, x1, x2 - x1, dx, y2, y1, y2 - y1, dy);
    printf("X_dir: %c, Speed: %d\n", x_dir, x_speed);
    printf("Y_dir: %c, Speed: %d\n", y_dir, y_speed);
    printf("Slope: %f\n", slope);
    */
}

void alternative_PWM_control_init(void) {

    // Configure the pins ICP4 and ICP5 as input with pull-up
    
    DDRL &= ~((1 << PL1) | (1 << PL0)); // ICP5 & ICP4
    PORTL |= ((1 << PL1) | (1 << PL0));

    // Initialize input capture mode for timer 4
    TCCR4B |= ((1 << ICNC4) | (1 << ICES4)); //input noise cancel & selecting rising edge for input capture
    TCCR4B |= ((1 << CS42)|(1 << CS40)); //setting prescaler of 1024

    // Initialize input capture mode for timer 5
    TCCR5B |= ((1 << ICNC5) | (1 << ICES5)); //input noise cancel & selecting rising edge for input capture
    TCCR5B |= ((1 << CS52)|(1 << CS50)); //setting prescaler of 1024


    // Enable Input Capture Interrupt and the Timer 1 Overflow Interrupt
    TIMSK4 |= ((1 << ICIE4) | (1 << TOIE4));

    // Enable Input Capture Interrupt and the Timer 5 Overflow Interrupt
    TIMSK5 |= ((1 << ICIE5) | (1 << TOIE5));

}

void alternative_PWM_control(unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {

    unsigned char motor_A_PWM = 0;
    unsigned char motor_B_PWM = 0;
    double slope;
    double x_distance = x2 - x1;
    double y_distance = y2 - y1;

    // Determine slope
    
    if(!((x_distance == 0.0) || (y_distance == 0.0))) { 
        slope = (y_distance / x_distance);
        slope = fabs((y_distance / x_distance));
    }
    else {
        if(x_distance == 0.0) {
            slope = 1000000;
        }
        if(y_distance == 0.0) {
            slope = 0.0;
        }
    }

    // Determine directions    
    char x_direction = copysign(1.0, x_distance);
    //printf("\nX-direction: %d", x_direction);

    if(x_direction == 1) {
        PWM_T3A_direction_change(1);
    }
    else {
        PWM_T3A_direction_change(0);
    }

    char y_direction = copysign(1.0, x_distance);
    //printf("\nY-direction: %d", y_direction);
    if(y_direction == 1) {
        PWM_T3B_direction_change(1);
    }
    else {
        PWM_T3B_direction_change(0);
    }

    // Set start PWM - Motor A: X Motor B: Y
    if(slope < 1000000) {
        motor_A_PWM = 60;
        PWM_T3A_set(motor_A_PWM);
    }
    if(slope != 0.0) {
        motor_B_PWM = 60;
        PWM_T3B_set(motor_A_PWM);
    }

    while((fabs(current_x_distance) <= x_distance) && (fabs(current_y_distance) <= y_distance)) {
        if((motor_B_PWM + 20) <= 255) {
            if((axisspeed_motor_B < (axisspeed_motor_A * slope)) || ((axisspeed_motor_B == 0.0) && (slope != 0.0))) {
                motor_B_PWM += 20;
            }
        }
        if((motor_B_PWM - 20) >= 0){
            if((axisspeed_motor_B > (axisspeed_motor_A * slope))) {
                motor_B_PWM -= 20;
            }
        }
        if((motor_A_PWM + 20) <= 255) {
            if((axisspeed_motor_A < (axisspeed_motor_B / slope)) ||  ((axisspeed_motor_A == 0.0) && (slope != 1000000))) {
                motor_A_PWM += 20;
            }
        }
        if((motor_A_PWM - 20) >= 0){
            if((axisspeed_motor_A > (axisspeed_motor_B / slope))){
                motor_A_PWM -= 20;
            }
        }
        PWM_T3A_set(motor_A_PWM);
        PWM_T3B_set(motor_B_PWM);
    }
    
    // Reset distances 
    current_x_distance = 0;
    current_y_distance = 0;
}

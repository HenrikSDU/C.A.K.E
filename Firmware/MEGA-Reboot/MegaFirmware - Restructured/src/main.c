#define BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU/(BAUDRATE*16UL)))-1)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>

#include "usart.h"

#include "file_proccessing.h"
#include "motorandgpio.h"

typedef enum {

    memory_init, // Preparing for incoming file
    upload, // Upload of file
    main_operation, // Creation of icing
    paused // Paused from main

}programstate_e;

void usart_send(unsigned char); // Function to send bytes via usart


volatile unsigned char memory_init_flags[10]; // Array to store information about incoming file send by PC
volatile unsigned int filesize = 0; // Variable to keep track of the number of bytes that will be send by the PC
volatile unsigned int instruction_count = 0;
volatile unsigned int interrupt_count = 0, file_index = 0;
volatile programstate_e phase = memory_init; // Phase indicating operation phase: memory_init, upload or main_operation


volatile bool readcycle_complete = false; // Read cycle complete
volatile int timer4overflow_count = 0;
volatile int timer5overflow_count = 0;
volatile double axisspeed_motor_A = 0;
volatile double current_x_distance = 0;
volatile double axisspeed_motor_B = 0;
volatile double current_y_distance = 0;

volatile bool a_origin_found = false;
volatile bool b_origin_found = false;

volatile float dx_pos = 0.0; // Used for tracking current position of the linear actuator
volatile float dy_pos = 0.0;

volatile char file[SUPPORTEDFILESIZE]; // Saves the incoming bytes from the computer 

//CAKEFILE cakefile; // Contains an array of instructions and points (table_instruction(s)) and an array that indicates whether the data saved at a specific index is a coordinate or an extruder instruction
g_instruction_t g_instructions[MAXIMUMINSTRUCTIONCOUNT];

ISR(USART0_RX_vect) {
    
    switch(phase) {

        case upload: // In the upload phase the incoming bytes are stored in the file array (a simple array of chars)
            file[file_index] = UDR0;
            file_index++; // Can maybe be replaced by i - pretty sure

        break;
        case memory_init:
            memory_init_flags[interrupt_count] = UDR0;
            if(interrupt_count >= 9){ // On 10th recieved byte indicate rcy_complete
                interrupt_count = 0; 
                readcycle_complete = true;
            }
        break;


    }    
    interrupt_count++; // Increment number of interrupts
    
}

ISR(PCINT2_vect) {

    if((PINK & (1 << BUTTON7)) == 0) {
        TOGGLE_ONBOARD_LED

        if(phase == paused) {
            phase = main_operation;
        } else {
            phase = paused;
        }
        _delay_ms(200);
    }

    if((PINK & (1 << BUTTON0)) == 0) {

        // Indicate successful zeroing on axis a
        b_origin_found = true; 

    }

    if((PINK & (1 << BUTTON7)) == 0) {

        // Indicate successful zeroing on axis b
        a_origin_found = true;

    }
   
}

ISR(TIMER5_OVF_vect) {

    timer5overflow_count++;
    TOGGLE_ONBOARD_LED
}

ISR(TIMER5_CAPT_vect) { // heeey

    axisspeed_motor_A = TICKDISTANCE / (double)(ICR1 + 0xFFFF * timer5overflow_count);
    current_y_distance += TICKDISTANCE;

    timer5overflow_count = 0;
    
    TOGGLE_ONBOARD_LED

    dx_pos = dx_pos + TICKDISTANCE;
}

ISR(TIMER4_OVF_vect) {

    timer4overflow_count++;

}

ISR(TIMER4_CAPT_vect) {

    axisspeed_motor_B = TICKDISTANCE / (double)(ICR3 + 0xFFFF * timer4overflow_count);
    current_x_distance += TICKDISTANCE;

    timer4overflow_count = 0;
    TOGGLE_ONBOARD_LED 

    dy_pos = dy_pos + TICKDISTANCE;
}


int main(void) { 

    uart_init();
    io_redirect();

    // Custom function initialization
    button_init();
    PWM_T3AB_init();
    PWM_T3C_init();
    //alternative_PWM_control_init();
    
    // Configuration of the IO pins

   
    // Enabling RX interrupt
    UCSR0B |= (1 << RXCIE0);
    sei(); // Enable interrupts globaly

    while(1) {
        
    
        while(phase == memory_init) {
        
            if(readcycle_complete) { // Handle memoryinitflags if recievecycle is complete
                
                //PORTD |= (1 << PD7); //indicate that recievecycle is complete

                filesize = memory_init_flags[0] * 255 + memory_init_flags[1]; // Compute filesize
                /*
                if(filesize > 0) { // Here size constraint possible
                    file = (char*)malloc(filesize * sizeof(unsigned char)); // Allocate memory for incoming CAKE-file
                    
                    if(file != NULL) {
                        //PORTD |= (1 << PD6); // Indicate successful memory allocation
                    }
                    
                }
                */

                instruction_count = memory_init_flags[2] * 255 + memory_init_flags[3]; // Getting the amount of instructions

                    //cakefile.path = (table_instruction*)calloc(instruction_count * sizeof(table_instruction), sizeof(table_instruction));
                    //cakefile.instruction_locations = (bool*)calloc(instruction_count * sizeof(bool), sizeof(bool));

                // Sending feedback 
                for(unsigned char j = 0; j < 10; j++)
                    usart_send(memory_init_flags[j]);

                // Resetting readcycle complete flag
                readcycle_complete = false;

                interrupt_count = 0;

                // Going over to next phase
                phase = upload;
            }
            
        }
        

        while(phase == upload) {
        
            //PORTB |= (1 << PB5);

            if((interrupt_count) >= filesize) { // Checking for successful upload

                interrupt_count = 0; // Resetting interrupt count
                //PORTD |= (1 << PD4);
                
                for(int k = 0; k < filesize; k++) // Send file back for feedback
                    usart_send(file[k]);

                UCSR0B &= ~(1 << RXCIE0); // Disable rx-interrupt

                file_processing(g_instructions, file, filesize); // Proccessing the recieved array
                             
                printf("FS:%d\n", filesize);
                for(int k = 0; k < instruction_count; k++) {

                    printf("G%d X%d Y%d\n", g_instructions[k].g_command, g_instructions[k].point.x, g_instructions[k].point.y);
                    
                }
                
                
                phase = main_operation;

            }
            

        }


        while(phase == main_operation) {


            // Initialization of Table
            PWM_CONTROL_RETURN PWM_control_feedback = {0};

            alternative_PWM_control_init();
            
            PWM_T3AB_init();

            // origin_function();
    
            PWM_T3A_set(0);
            PWM_T3B_set(0);

            // Control Section


            _delay_ms(10000);
            ////////////////////////
            //cli();
            ///////////////////////
            //printf("B3:setB4:incB5:decB6:dich");
        }

        uint8_t direction  = 0;
        uint8_t desired_PWM = 0;
        uint8_t motorA_PWM = 0;
        uint8_t motorB_PWM = 0;
        uint8_t motorC_PWM = 0;

        while(phase == paused) {
            

                if((PINK & (1 << BUTTON3)) == 0) { // Red cable

                    PWM_T3A_set(motorA_PWM);
                    PWM_T3B_set(motorB_PWM);
                    PWM_T3C_set(motorC_PWM);
                    TOGGLE_ONBOARD_LED
                    
                    
                    _delay_ms(100);
                }

                if((PINK & (1 << BUTTON4)) == 0) { // Green Cable
                    motorA_PWM += 10;
                    //printf("desPWM: %u ", desired_PWM);
                    TOGGLE_ONBOARD_LED
                    _delay_ms(100);
                }
                if((PINK & (1 << BUTTON5)) == 0) { // White Cable

                    motorA_PWM -= 10;
                    TOGGLE_ONBOARD_LED
                    //printf("desPWM: %u ", desired_PWM);
                    _delay_ms(100);
                }
                if((PINK & (1 << BUTTON6)) == 0) { // Grey Cable
                    direction = 0b00000001 & direction;
                    PWM_T3A_direction_change(direction);
                    PWM_T3B_direction_change(direction);
                    PWM_T3C_direction_change(direction);
                    //printf("dir:%d",direction);
                    TOGGLE_ONBOARD_LED
                    direction++;
                    _delay_ms(500);
                }
                if((PINB & (1 << BUTTON8)) == 0) { // Yellow Cable
                    motorB_PWM += 10;
                }
                if((PINB & (1 << BUTTON9)) == 0) { // Purple Cable
                    motorB_PWM = 255;
                    motorA_PWM = 255;
                    motorC_PWM = 255;

                }       

            
        }
    
    }
    return 0;
}


void usart_send(unsigned char data) { 
    // Check whether there is space in the sending buffer
    while(!(UCSR0A & (1 << UDRE0))); // Wait for transmit buffer
    UDR0 = data;
}

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



#include "lcd.h"
#include "usart.h"
#include "i2cmaster.h"
#include "lm75.h"

#include "file_proccessing.h"
#include "motorandgpio.h"

typedef enum{

    memory_init, //preparing for incoming file
    upload, //upload of file
    main_operation, //creation of icing
    paused //paused from main

}programstate_e;




void usart_send(unsigned char); // Function to send bytes via usart


volatile unsigned char memory_init_flags[10]; // Array to store information about incoming file send by PC
volatile unsigned int filesize = 0; // Variable to keep track of the number of bytes that will be send by the PC
volatile unsigned int instruction_count = 0;
volatile unsigned int interrupt_count = 0, file_index = 0;
volatile programstate_e phase = memory_init; // Phase indicating operation phase: memory_init, upload or main_operation
volatile bool readcycle_complete = false; // Read cycle complete

volatile char file[SUPPORTEDFILESIZE]; // Saves the incoming bytes from the computer 

CAKEFILE cakefile; // Contains an array of instructions and points (table_instruction(s)) and an array that indicates whether the data saved at a specific index is a coordinate or an extruder instruction

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
        } else if(phase == main_operation) {
            phase = paused;
        }
        _delay_ms(100);
    }
   

}

int main(void) { 

    i2c_init();// Initialization of the I2C communication
    
    //LCD_init();// Initialization of the LCD
    //lm75_init();// Initialization of the temperature sensor
    
    uart_init();
    io_redirect();

    // Custom function initialization
    button_init();

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
            int k;
            //PORTB |= (1 << PB5);

            if((interrupt_count) >= filesize) { // Checking for successful upload

                interrupt_count = 0; // Resetting interrupt count
                //PORTD |= (1 << PD4);
                
                for(k = 0; k < filesize; k++) // Send file back for feedback
                    usart_send(file[k]);

                UCSR0B &= ~(1 << RXCIE0); // Disable rx-interrupt
                file_processing(&cakefile, file, filesize); // Proccessing the recieved array
                
                //LCD_init();
                //LCD_set_cursor(0,0);
                
                printf("FS:%d", filesize);
                for(k = 0; k < instruction_count; k++) {

                // LCD_set_cursor(0, (k%4));
                    if(cakefile.instruction_locations[k] == 1) {
                        printf("G%d", cakefile.path[k].extruder_inst + 1);
                    }
                    else {
                        printf("X:%dY:%d ", cakefile.path[k].table_coord.x, cakefile.path[k].table_coord.y);
                    }
                    //_delay_ms(500);
                }
                
                
                phase = main_operation;


            }
            

        }


        while(phase == main_operation) {
            
            
            PWM_T4AB_init();
            PWM_T4A_set(200);
            _delay_ms(5000);
            PWM_T4A_direction_change(1);
            PWM_T4A_set(0);
            _delay_ms(3000);
            
            //LCD_init();
            //LCD_set_cursor(0,0);

            unsigned char desired_PWM = 100;
            unsigned char x_array[] = {2, 2, 50, 100, 60, 50, 0, 1};
            unsigned char y_array[] = {2, 2, 50, 120, 140, 10, 0, 200};

            for(int i = 0; i < 7; i++) {
                PWM_control(desired_PWM, x_array[i], x_array[i + 1], y_array[i], y_array[i + 1]);
                _delay_ms(1000);
            }

            for(int print_index = 0; print_index < 7; print_index++) {

                /* From the internet:     also half of this file is empty lines
                Disadvantages of Using goto Statement
                The use of the goto statement is highly discouraged as it makes the program logic very complex.
                The use of goto makes tracing the flow of the program very difficult.
                The use of goto makes the task of analyzing and verifying the correctness of programs (particularly those involving loops) very difficult.
                */
               // Here the goto would not be a main component of the program logic as it would only be called if desired so by the user
               // It would just serve as a reentry point when jumping back from paused - we could implement a same functionality with global variables of course
               // But I think in this case that would be more complex then a goto

            reentrypoint: // If we paused the print we can use this goto to continue in the print - maybe
                // also also i think that having this seprarate instruction for the g commands is going to be bad here, bc more execptions will need to be handled

                // But how do we want to handle the g commands else? Also due to the elses the conditions only get checked occacionally thus not slowing down the process much

                //also the reason this didnt work was because the if condition wasnt applicable to a normal array, only cakefile
                if((array[print_index] == 0) && (array[print_index + 1] == 0)) {
                    PWM_control(desired_PWM, array[print_index], array[print_index + 1], 100, 150);
                    _delay_ms(1000);
                }
                else {
                    if(cakefile.path[print_index].extruder_inst== G1) {

                        // Execute G1

                    }
                    else                    
                    if(cakefile.path[print_index].extruder_inst == G2) {

                        // Execute G2

                    }

                }
                

            }

            ////////////////////////
            //cli();
            ///////////////////////

            //printf("B3:setB4:incB5:decB6:dich");
            
           
            
        }

        while(phase == paused){

            TOGGLE_ONBOARD_LED
            _delay_ms(1000);
            PWM_T4A_set(0); 

            uint8_t direction = 0;
            uint8_t desired_PWM = 100;
            while(phase != paused) {

                if((PINK & (1 << BUTTON3)) == 0) {

                    PWM_T4A_set(desired_PWM);
                    TOGGLE_ONBOARD_LED
                    
                    
                    _delay_ms(200);
                }

                if((PINK & (1 << BUTTON4)) == 0) {
                    desired_PWM += 50;
                    //LCD_set_cursor(0,2);
                    //printf("desPWM: %u ", desired_PWM);
                    TOGGLE_ONBOARD_LED
                    _delay_ms(200);
                }
                if((PINK & (1 << BUTTON5)) == 0) {

                    desired_PWM -= 50;
                    TOGGLE_ONBOARD_LED
                    //LCD_set_cursor(0,2);
                    //printf("desPWM: %u ", desired_PWM);
                    _delay_ms(200);
                }
                if((PINK & (1 << BUTTON6)) == 0) {
                    direction = 0b00000001 & direction;
                    PWM_T4A_direction_change(direction);
                    //LCD_set_cursor(0,3);
                    //printf("dir:%d",direction);
                    TOGGLE_ONBOARD_LED
                    direction++;
                    _delay_ms(2000);
                }

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

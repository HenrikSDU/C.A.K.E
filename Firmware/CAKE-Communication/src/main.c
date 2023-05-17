//The CAKE communication
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

/* Macros for GPIO pins */
#define FORWARD_PIN PORTD2
#define BACKWARD_PIN PORTD3


#include "file_proccessing.h"
#include "motorandgpio.h"




void usart_send(unsigned char); //function to send bytes via usart


volatile unsigned char memory_init_flags[10]; //array to store information about incoming file send by PC
volatile unsigned int filesize = 0; //variable to keep track of the number of bytes that will be send by the PC
volatile unsigned int instruction_count = 0;
volatile unsigned int interrupt_count = 0, file_index = 0;
volatile programstate_e phase = memory_init; //phase indicating operation phase: memory_init, upload or main_operation
volatile bool readcycle_complete = false; //read cycle complete

volatile char* file; //saves the incoming bytes from the computer 

CAKEFILE cakefile; //contains an array of instructions and points (table_instruction(s)) and an array that indicates whether the data saved at a specific index is a coordinate or an extruder instruction

ISR(USART_RX_vect) {
    
    switch(phase) {

        case upload://in the upload phase the incoming bytes are stored in the file array (a simple array of chars)
            file[file_index] = UDR0;
            file_index++;//can maybe be replaced by i - pretty sure

        break;
        case memory_init:
            memory_init_flags[interrupt_count] = UDR0;
            if(interrupt_count >= 9){//on 10th recieved byte indicate rcy_complete
                interrupt_count = 0; 
                readcycle_complete = true;
            }
        break;


    }    
    interrupt_count++;//increment number of interrupts
    
}

ISR(PCINT1_vect) {

    if((PINC & (1 << BUTTON4)) == 0)
        PORTB ^= (1 << PB5);

}

int main(void) { 

    i2c_init();// Initialization of the I2C communication
    
    //LCD_init();// Initialization of the LCD
    //lm75_init();// Initialization of the temperature sensor
    
    uart_init();
    io_redirect();

    // Configuration of the IO pins

    DDRC |= 0x30; // For I2C
    PORTC |= 0x30;
    DDRB |= (1 << PB5);

   
    // Enabling RX interrupt
    UCSR0B |= (1 << RXCIE0);
    sei(); // Enable interrupts globaly

    while(phase == memory_init) {
       
        if(readcycle_complete) { // Handle memoryinitflags if recievecycle is complete
            
            //PORTD |= (1 << PD7); //indicate that recievecycle is complete

            filesize = memory_init_flags[0] * 255 + memory_init_flags[1]; // Compute filesize

            if(filesize > 0) { // Here size constraint possible
                file = (char*)malloc(filesize * sizeof(unsigned char)); // Allocate memory for incoming CAKE-file
                
                if(file != NULL) {
                    //PORTD |= (1 << PD6); // Indicate successful memory allocation
                }
                
            }

            instruction_count = memory_init_flags[2]; // Getting the amount of instructions

                cakefile.path = (table_instruction*)calloc(instruction_count * sizeof(table_instruction), sizeof(table_instruction));
                cakefile.instruction_locations = (bool*)calloc(instruction_count * sizeof(bool), sizeof(bool));

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

        if((interrupt_count) >= filesize) { // Checking for successful upload

            interrupt_count = 0; // Resetting interrupt count
            //PORTD |= (1 << PD4);
            
            for(k = 0; k < filesize; k++) // Send file back for feedback
                usart_send(file[k]);

            UCSR0B &= ~(1 << RXCIE0); // Disable rx-interrupt

            file_processing(&cakefile, file, filesize); // Proccessing the recieved array

            LCD_init();
            LCD_set_cursor(0,0);
            printf("FS:%d", filesize);
            for(k = 0; k < instruction_count; k++) {

                LCD_set_cursor(0, (k%4));
                if(cakefile.instruction_locations[k] == 1) {
                    printf("G%d", cakefile.path[k].extruder_inst + 1);
                }
                else {
                    printf("X:%dY:%d ", cakefile.path[k].table_coord.x, cakefile.path[k].table_coord.y);
                }
                _delay_ms(10);
            }

            
            phase = main_operation;


        }
        

    }

    while(phase == main_operation) {
        
        button_init();
        LCD_init();
        LCD_set_cursor(0,0);


        // Initialization for IOBoard - speed measurement
        
        DDRB |= (1<<PB5);

        PWM_T0A_init();
        char desired_PWM = 0;

        ////////////////////////
        cli();
        ///////////////////////

        printf("B3:setB4:incB5:decB6:dich");
        char direction;
        while(1) {

            if((PINC & (1 << BUTTON3)) == 0) {

                PWM_T0A_set(desired_PWM);
                
                _delay_ms(200);
            }

            if((PINC & (1 << BUTTON4)) == 0) {
                desired_PWM += 10;
                LCD_set_cursor(0,2);
                printf("desPWM: %u ", desired_PWM);
                _delay_ms(200);
            }
            if((PINC & (1 << BUTTON5)) == 0) {

                desired_PWM -= 10;
                LCD_set_cursor(0,2);
                printf("desPWM: %u ", desired_PWM);
                _delay_ms(200);
            }
            if((PINC & (1 << BUTTON6)) == 0) {
                direction = 0b00000001 & direction;
                PWM_T0A_direction_change(direction);
                LCD_set_cursor(0,3);
                printf("dir:%d",direction);
                direction++;
                _delay_ms(200);
            }



        }


    }
    
    return 0;
}


void usart_send(unsigned char data) { 
    //check whether there is space in the sending buffer
    while(!(UCSR0A & (1 << UDRE0)));//wait for transmit buffer
    UDR0 = data;

}



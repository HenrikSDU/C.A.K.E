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





void usart_send(unsigned char); //function to send bytes via usart
unsigned char usart_receive(void);
void reset_reciever(void); //function to reset variables associated with the communication 


//moving functions


volatile unsigned char memory_init_flags[10]; //array to store information about incoming file send by PC
volatile unsigned int filesize = 0; //variable to keep track of the number of bytes that will be send by the PC
volatile unsigned int instruction_count = 0;
volatile unsigned int interrupt_count = 0, file_index = 0;
volatile programstate_e phase = memory_init; //phase indicating operation phase: memory_init, upload or main_operation
volatile bool readcycle_complete = false; //read cycle complete

volatile char* file; //saves the incoming bytes from the computer 

CAKEFILE cakefile; //contains an array of instructions and points (table_instruction(s)) and an array that indicates whether the data saved at a specific index is a coordinate or an extruder instruction

ISR(USART_RX_vect){
    
    switch(phase){

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

int main(void) { 


    /*
    char ramtest[500];
    memset(ramtest, 0, 500); // Initialize memory to a certain value
    for(int u = 0; u < 500; u++){
        ramtest[u] = 5;
        printf("%d",ramtest[u]+4);
    }
    */
    i2c_init();//initialize I2C communication
    
    //LCD_init();//initialize the LCD
    lm75_init();//initialize the temperature sensor
    
    uart_init();
    io_redirect();

    //configuration of the IO pins

    DDRD = 0xFF;
    PORTD = 0x00;
    DDRC = 0xF0;
    PORTC = 0x3F;
    DDRB |= (1<<PB5);

   
    //enable RX interrupt
    UCSR0B |= (1 << RXCIE0);
    sei(); //enable interrupts globaly

    while(phase == memory_init){
        if(PINC == 0b00111110){
            for(unsigned char j = 0; j < 10; j++)
            usart_send(memory_init_flags[j]);
            _delay_ms(250);
        }
        if(PINC == 0b00111101){
            reset_reciever();
            _delay_ms(250);
        }
        if(readcycle_complete){//handle memoryinitflags if recievecycle is complete
            
            PORTD |= (1 << PD7); //indicate that recievecycle is complete

            filesize = memory_init_flags[0] * 255 + memory_init_flags[1]; //compute filesize

            if(filesize > 0){ //here size constraint possible
                file = (char*)malloc(filesize*sizeof(unsigned char)); //allocate memory for incoming CAKE-file
                
                if(file != NULL){
                    PORTD |= (1 << PD6); //indicate successful memory allocation
                }
                
            }

            instruction_count = memory_init_flags[2]; //getting the amount of instructions

                cakefile.path = (table_instruction*)calloc(instruction_count * sizeof(table_instruction), sizeof(table_instruction));
            //sending feedback 
            for(unsigned char j = 0; j < 10; j++)
                usart_send(memory_init_flags[j]);

            //resetting readcycle complete flag
            readcycle_complete = false;

            interrupt_count = 0;

            //going over to next phase
            phase = upload;
        }
        
    }
    

    while(phase == upload){
        int k;
        //_delay_ms(5000);
        if(filesize == 245)
        PORTD |= (1 << PD5);

        if((interrupt_count) >= filesize){//checking for successful upload

            UCSR0B &= ~(1 << RXCIE0);//disable rx-interrupt
            interrupt_count = 0; //resetting i
            PORTD |= (1 << PD4);
            for(k = 0; k < filesize; k++)//send file back for feedback
            usart_send(file[k]);

            file_processing(&cakefile, file, filesize);//proccessing the recieved array

        

            //phase = main_operation;


        }
        if(PINC == 0b00111110){
            //for(k = 0; k < filesize; k++)//send file back for feedback
            //usart_send(file[k]);
        }
        if(PINC == 0b00111101){
            usart_send(interrupt_count);
            _delay_ms(500);
            
        }
        if(PINC == 0b00111011){
            PORTD |= (1 << PD4);
            LCD_init();
            LCD_set_cursor(0,0);
            printf("i:%d fs:%d E1:%c", file_index, filesize, file[0]);
            printf("E2:%c E3:%c", file[1], file[filesize - 1]);
            printf("afs:%d", k);
            LCD_set_cursor(0, 3);
            _delay_ms(5000);
            LCD_clear();
            for(k = 0; k < filesize; k++){
                printf("%c", file[k]);
                _delay_ms(500);
            }          
            uart_init();
            io_redirect();
            _delay_ms(500);
        }

        if(PINC == 0b00110111){

            //file_processing(&cakefile, file, filesize);
            LCD_init();
            LCD_set_cursor(0,0);
            printf("FS:%d", filesize);
            for(k = 0; k < instruction_count; k++){

                LCD_set_cursor(0, (k%4));
                if(cakefile.instruction_locations[k] == 1){
                    printf("G%d", cakefile.path[k].extruder_inst+1);
                }
                else{
                    printf("X:%dY:%d ", cakefile.path[k].table_coord.x, cakefile.path[k].table_coord.y);
                }
                _delay_ms(1500);
            }

            uart_init();
            io_redirect();
            _delay_ms(500);

        }

    }

    while(phase == main_operation){
        PORTD = 0x0F; //indicate that main operation has been reached
        

    }
    
    return 0;
}


void usart_send(unsigned char data){ 
    //check whether there is space in the sending buffer
    while(!(UCSR0A & (1 << UDRE0)));//wait for transmit buffer
    UDR0 = data;

}

unsigned char usart_receive(void){
  
    //check whether there is something newly received
    while(!(UCSR0A & (1 << RXC0)));
    
    return UDR0;//return received data
}

void reset_reciever(void){
    
    interrupt_count = 0;
    phase = memory_init;

}

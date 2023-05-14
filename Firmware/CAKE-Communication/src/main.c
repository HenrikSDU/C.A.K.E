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



typedef struct{
    uint8_t x;//x coordinate
    uint8_t y;//y coordinate
}coordinate;

typedef enum{
    G1,
    G2, 
    G3 
}extruder_instruction;

/*a union is a kind of structure. However, a union, unlike a structure, has always only one of its variables at the same time.
 For example, for the union here: If you define exampletableinstruction.extruder_inst = G1; than
 you will not be able to read any useful data from its table_coord variable. Conversly, when you define its table_coord component, 
 you overwrite its extruder_inst component.
 A union alway just holds one of its components. It like a variable that can store two different data types. 
*/
typedef union{

    coordinate table_coord;
    extruder_instruction extruder_inst;

}table_instruction;

typedef struct{

    table_instruction path[300];
    bool instruction_locations[300];

}CAKEFILE;

typedef enum{

    memory_init, //preparing for incoming file
    upload, //upload of file
    main_operation //creation of icing

}programstate_e;


void usart_send(unsigned char); //function to send bytes via usart
unsigned char usart_receive(void);
void reset_reciever(void); //function to reset variables associated with the communication 

void file_processing(void);//function extracting information like coordinates and extruder instructions and storing it in the CAKEFILE structure


volatile unsigned char memory_init_flags[10]; //array to store information about incoming file send by PC
volatile unsigned int filesize = 0; //variable to keep track of the number of bytes that will be send by the PC
volatile unsigned int i = 0, file_index = 0;
volatile programstate_e phase = memory_init; //phase indicating operation phase: memory_init, upload or main_operation
volatile bool rcy_complete = false; //read cycle complete

volatile char* file; //saves the incoming bytes from the computer 

CAKEFILE cakefile; //contains an array of instructions and points (table_instruction(s)) and an array that indicates whether the data saved at a specific index is a coordinate or an extruder instruction

ISR(USART_RX_vect){
    
    switch(phase){

        case upload://in the upload phase the incoming bytes are stored in the file array (a simple array of chars)
            file[file_index] = UDR0;
            file_index++;//can maybe be replaced by i - pretty sure

        break;
        case memory_init:
            memory_init_flags[i] = UDR0;
            if(i >= 9){//on 10th recieved byte indicate rcy_complete
                i = 0; 
                rcy_complete = true;
            }
        break;


    }    
    i++;//increment index
    
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
        if(rcy_complete){//handle memoryinitflags if recievecycle is complete
            
            PORTD |= (1 << PD7); //indicate that recievecycle is complete

            filesize = memory_init_flags[0];
            if(filesize < 255){
                file = (char*)malloc((unsigned char)filesize*sizeof(unsigned char)); //allocate memory for incoming CAKE-file
                
                if(file != NULL){
                    PORTD |= (1 << PD6); //indicate successful memory allocation
                }
                
            }

            //sending feedback 
            for(unsigned char j = 0; j < 10; j++)
                usart_send(memory_init_flags[j]);

            //resetting readcycle complete flag
            rcy_complete = false;

            //going over to next phase
            phase = upload;
        }
        
    }
    

    while(phase == upload){
        int k;
        //_delay_ms(5000);
        if(filesize == 245)
        PORTD |= (1 << PD5);

        if((i) >= filesize){//checking for successful upload

            UCSR0B &= ~(1 << RXCIE0);//disable rx-interrupt
            i = 0; //resetting i
            PORTD |= (1 << PD4);
            for(k = 0; k < filesize; k++)//send file back for feedback
            usart_send(file[k]);

            file_processing();//proccessing the recieved array


        }
        if(PINC == 0b00111110){
            //for(k = 0; k < filesize; k++)//send file back for feedback
            //usart_send(file[k]);
        }
        if(PINC == 0b00111101){
            usart_send(i);
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
            printf("FS:%d", filesize);
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

            file_processing();
            LCD_init();
            LCD_set_cursor(0,0);
            for(k = 0; k < filesize; k++){
                if(cakefile.instruction_locations[k] == 1){
                    printf("G%d", cakefile.path[k].extruder_inst+1);
                }
                else{
                    printf("X:%d Y:%d;", cakefile.path[k].table_coord.x, cakefile.path[k].table_coord.y);
                }
                _delay_ms(1500);
            }

            uart_init();
            io_redirect();
            _delay_ms(500);

        }

    }

    while(phase == main_operation){


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

    i = 0;
    phase = memory_init;

}

void file_processing(void){
    /*
    This function has to deal with the problem of extracting data from the recieved array of characters/bytes. On the PC the information 
    is saved in ASCII, even the numbers. For example, 15 would not be send as 0b00001111, but as first a 1 and then a 5
    in ASCII. That means first 1 in ASCII: 0b00110001 and then a 5, also in ASCII: 0b00110101; 
    This function checks whether in this array of characters for extruder instructions and coordinates and saves them in the CAKEFILE structure.
    Observe, its table instruction component is a union. The array of instruction_locations then indicates whether the element saved
    at a specific location is an extruder_instruction or a coordinate. 1 means extruder instruction, 0 means coordinate.
    For example, when later reading the CAKEFILE cakefile, when cakefile.instruction_locations[x] == 1, then cakefile.table_instructions[x] is an instruction.
    Conversly, if cakefile.instruction_locations[x] == 0 then cakefile.table_instructions[x] is a coordinate.

    An ASCII table:
    https://www.sciencebuddies.org/science-fair-projects/references/ascii-table
    */

    LCD_init();
    LCD_set_cursor(0,0);
    int savelocation = 0; //location in the CAKEFILE that is written to
    for(int read_index = 0; read_index < filesize; read_index++){

        if(file[read_index] == 'G'){
            
            cakefile.instruction_locations[savelocation] = 1; //a one indicates that at the position a instruction is saved

            cakefile.path[savelocation].extruder_inst = (file[read_index + 1] & 0x0F) - 1;//extracts the instruction  -- atoi() -
            
            //read_index++;//handeling that the instruction is two bytes

            savelocation++;//incrementing savelocation in order not to overwrite the old instruction when a new one is extracted from the array

            //printf("G%d", cakefile.path[savelocation].extruder_inst);
        }
        else
        if(file[read_index] == 'X'){//check for the beginning of a coordinate block - they all are in the form of X(coordinate in ASCII)Y(coordinate in ASCII)
            
            cakefile.instruction_locations[savelocation] = 0;//a one indicates that at the position no instruction is saved

            //extracting the coordinates
            //X-coordinate
            int string_scanner = 1;//variable to "look ahead" in the string without incrementing read_index
            cakefile.path[savelocation].table_coord.x = 0; //initializing the coorinate to zero
            while(file[read_index + string_scanner] != 'Y'){
                
                cakefile.path[savelocation].table_coord.x *= 10;//shifting the previous number one decimal to the side
                cakefile.path[savelocation].table_coord.x += (file[read_index + string_scanner] & 0x0F);//adding the new number
                
                string_scanner++;//increment in order to look one element further in the file array the next time the while-loop starts
                //printf(" %d ", file[read_index + string_scanner] & 0x0F);
            } 
            //updating read_index to start at Y-coordinate
            read_index += string_scanner; //file[read_index] should now be equal to 'Y'

            //printf("X:%d", cakefile.path[savelocation].table_coord.x);
            
            //Y-coordinate
            string_scanner = 1;//reseting string scanner variable
            cakefile.path[savelocation].table_coord.y = 0; //initializing the coorinate to zero
            while((file[read_index + string_scanner] & 0b00110000)){//checking whether there a numbers hidden in ASCII - the expression comes from the ASCII characteristics - numbers are saved like: 0b0011xxxx with the x's being the actual number; new line characters are 0b0000xxxx 
                
                cakefile.path[savelocation].table_coord.y *= 10;//shifting the previous number one decimal to the side
                cakefile.path[savelocation].table_coord.y += (file[read_index + string_scanner] & 0x0F);//adding the new number
                
                string_scanner++;//increment in order to look one element further in the file array the next time the while-loop starts
            }

            //printf("Y:%d", cakefile.path[savelocation].table_coord.y);

            //updating read_index in order not to read the chordinates again
            //read_index += string_scanner; //file[read_index] should now be equal to newline character

            savelocation++;//incrementing savelocation in order not to overwrite the old instruction when a new one is extracted from the array

        }
        
        //_delay_ms(150);


    }

    uart_init();
    io_redirect();
}
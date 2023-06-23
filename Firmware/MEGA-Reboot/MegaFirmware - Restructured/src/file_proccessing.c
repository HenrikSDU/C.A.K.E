//C.A.K.E File Processing header file
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

void file_processing(g_instruction_t* g_instructions, volatile char* file, int filesize){
    
    // This function has to deal with the problem of extracting data from the recieved array of characters/bytes. On the PC the information 
    // is saved in ASCII, even the numbers. For example, 15 would not be send as 0b00001111, but as first a 1 and then a 5
    // in ASCII. That means first 1 in ASCII: 0b00110001 and then a 5, also in ASCII: 0b00110101; 
    // This function checks whether in this array of characters for extruder instructions and coordinates and saves them in the CAKEFILE structure.
    // Observe, its table instruction component is a union. The array of instruction_locations then indicates whether the element saved
    // at a specific location is an extruder_instruction or a coordinate. 1 means extruder instruction, 0 means coordinate.
    // For example, when later reading the CAKEFILE cakefile, when cakefile.instruction_locations[x] == 1, then cakefile.table_instructions[x] is an instruction.
    // Conversly, if cakefile.instruction_locations[x] == 0 then cakefile.table_instructions[x] is a coordinate.

    // An ASCII table:
    // https://www.sciencebuddies.org/science-fair-projects/references/ascii-table


    extruder_instruction temp_extruder_instruction;
    int savelocation = 0; // Location in the g_instructions array that is written to
    for(int read_index = 0; read_index < filesize; read_index++) {

        if(file[read_index] == 'G') {

            g_instructions[savelocation].g_command = (file[read_index + 1] & 0x0F);
            temp_extruder_instruction = g_instructions[savelocation].g_command;

            printf("G_found: %d\n", g_instructions[savelocation].g_command);
                        
        }
        else
        if(file[read_index] == 'X') { // Check for the beginning of a coordinate block - they all are in the form of X(coordinate in ASCII)Y(coordinate in ASCII)

            // Extracting the coordinates
            // X-coordinate
            int string_scanner = 1;//variable to "look ahead" in the string without incrementing read_index
            
            g_instructions[savelocation].point.x = 0;
            while(file[read_index + string_scanner] != 'Y') {
                
                g_instructions[savelocation].point.x *= 10;
                g_instructions[savelocation].point.x += (file[read_index + string_scanner] & 0x0F);

                string_scanner++;//increment in order to look one element further in the file array -the next time the while-loop starts
                
            } 
            
            // Updating read_index to start at Y-coordinate
            read_index += string_scanner; //file[read_index] should now be equal to 'Y'

            // Y-coordinate
            string_scanner = 1; // Reseting string scanner variable
            
            g_instructions[savelocation].point.y = 0;
            while((file[read_index + string_scanner] & 0b00110000)) { // Checking whether there a numbers hidden in ASCII - the expression comes from the ASCII characteristics - numbers are saved like: 0b0011xxxx with the x's being the actual number; new line characters are 0b0000xxxx 
                
                g_instructions[savelocation].point.y *= 10;
                g_instructions[savelocation].point.y += (file[read_index + string_scanner] & 0x0F);

                string_scanner++;//increment in order to look one element further in the file array the next time the while-loop starts
            }

            
            //updating read_index in order not to read the chordinates again
            read_index += string_scanner; //file[read_index] should now be equal to newline character

            g_instructions[savelocation].g_command = temp_extruder_instruction;

            savelocation++;//incrementing savelocation in order not to overwrite the old instruction when a new one is extracted from the array

        }
        
    }
    
}
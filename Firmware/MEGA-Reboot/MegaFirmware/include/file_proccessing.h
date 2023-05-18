//C.A.K.E File proscessing header file

//refer to MATLAB SCRIPT
#define SUPPORTEDFILESIZE 4496
#define MAXIMUMINSTRUCTIONCOUNT 899


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
} coordinate;

typedef enum{
    G1,
    G2, 
    G3 
} extruder_instruction;

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

    table_instruction path[MAXIMUMINSTRUCTIONCOUNT]; //array of instructions and coordinates
    bool instruction_locations[MAXIMUMINSTRUCTIONCOUNT]; //array indicating whether element in path is a extruder_inst (1) or a coordinate (0)

}CAKEFILE;



void file_processing(CAKEFILE* cakefile, volatile char* file, int filesize);
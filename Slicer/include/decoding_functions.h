#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "instructions.h"

// Point struct
typedef struct {
    double x;
    double y;
} point_t;

// This function is used to initialize a point_t struct by setting all values to 0
point_t init_point(void) {
    point_t point;
    point.x = 0.0;
    point.y = 0.0;
    return point;
}

// Bezier curve struct
typedef struct {
    double start_x;
    double start_y;
    double c1_x;
    double c1_y;
    double c2_x;
    double c2_y;
    double end_x;
    double end_y;
} bezier_t;

// This function is used to initialize a bezier_t struct by setting all values to 0
bezier_t init_bezier() {
    bezier_t bezier;
    bezier.start_x = 0;
    bezier.start_y = 0;
    bezier.c1_x = 0;
    bezier.c1_y = 0;
    bezier.c2_x = 0;
    bezier.c2_y = 0;
    bezier.end_x = 0;
    bezier.end_y = 0;
    return bezier;   
}

// Enum to represent the possible instructions in the svg file
typedef enum {
    M,
    C,
    L,
} command_t;

// Enum to represent the possible kinds of data the program reads in d property
typedef enum {
    ERR,
    NUM,
    CHAR,
} byte_result;

// Custom struct to return a byte_result and an increment(to increment the cursor past the read values) value
typedef struct {
    byte_result result;
    int increment;
} find_return;

// This function is used to initialize a find_return struct by setting all values to 0
find_return find_return_init() {
    find_return temp;
    temp.result = ERR;
    temp.increment = 0;
    return temp;
}

/* Bézier-curve deciphering functions */
double cubic_bezier_x(double start_x, double c1_x, double c2_x, double end_x, double t) {
    double ratio = (double)1 - t;
    double x = start_x * pow(ratio, (double)3) + c1_x * (double)3*t * pow(ratio, (double)2) + c2_x * (double)3*pow(t, (double)2) * ratio + end_x * pow(t, (double)3);
    return x;
}

double cubic_bezier_y(double start_y, double c1_y, double c2_y, double end_y, double t) {
    double ratio = (double)1 - t;
    double y = start_y * pow(ratio, (double)3) + c1_y * (double)3*t * pow(ratio, (double)2) + c2_y * (double)3*pow(t, (double)2) * ratio + end_y * pow(t, (double)3);
    return y;
}


/* Small example for the str_shift_left function
    char test1[] = "asd";
    str_shift_left(test1, sizeof(test1), 'b');
    printf("Asd: %s\n", test1);
    this will print "bas" */ 
void str_shift_left(char* str, int length, char next) {
    char* temp = malloc((length - 1) * sizeof(char));
    for(int i = 1; i < length - 1 + 1; i++) {
        temp[i] = str[i - 1]; // temp = _as
    }
    temp[0] = next;
    for(int i = 0; i < length - 1; i++) {
        str[i] = temp[i];
    }

    free(temp);
}

// Finds where the string  of the d property starts
unsigned int str_find_d(char* str, unsigned int cursor, unsigned int length) {
    char d[4]; //The string we want to find, has to be inputted backwards except for the NULL char
    d[0] = 34; // "
    d[1] = 61; // =
    d[2] = 100; // d
    d[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) { // Iterates through the string
        str_shift_left(temp, sizeof(temp), str[i]); // Shifts the string to the left and adds the next char to the beginning

        match = 0;
        for(int j = 0; j < sizeof(d); j++) { // Compares the temporary string to the string we want to find
            //printf("the compared chars: %c, %c\n", d[j], temp[j]);
            if(d[j] == temp[j])
                match++;
        }

        if(match == 4) // If enough characters match, we have found the string
            return i + 1; // Returns the current "position" basically where the specified string ends + 1
        }
    return length; //Error
}

unsigned int str_find_width(char* str, unsigned int cursor, unsigned int length) { // Finds the width of the image
    char width[4]; //The string we want to finwidth, has to be inputtewidth backwarwidths except for the NULL char
    width[0] = 34; // "
    width[1] = 61; // =
    width[2] = 104; // h
    width[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) { // Iterates through the string
        str_shift_left(temp, sizeof(temp), str[i]); // Shifts the string to the left and adds the next char to the beginning
        match = 0;
        for(int j = 0; j < sizeof(width); j++) { // Compares the temporary string to the string we want to find
            if(width[j] == temp[j])
                match++;
        }
        if(match == 4) // If enough characters match, we have found the string
            return i + 1; // Returns the current "position" basically where the specified string ends + 1
        }
    return length; //Error return is the lenght of the file meaning it didn't find the width
}

unsigned int str_find_height(char* str, unsigned int cursor, unsigned int length) { // Finds the height of the image
    char height[4]; //The string we want to finwidth, has to be inputtewidth backwarwidths except for the NULL char
    height[0] = 34; // "
    height[1] = 61; // =
    height[2] = 116; // t
    height[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) { // Iterates through the string
        str_shift_left(temp, sizeof(temp), str[i]); // Shifts the string to the left and adds the next char to the beginning
        match = 0;
        for(int j = 0; j < sizeof(height); j++) { // Compares the temporary string to the string we want to find
            if(height[j] == temp[j])
                match++;
        }
        if(match == 4) // If enough characters match, we have found the string
            return i + 1; // Returns the current "position" basically where the specified string ends + 1
        }
    return length; //Error return is the lenght of the file meaning it didn't find the height
}

/* Example
    double t = 0;
    Usage of cubic bezier functions, if int i = 0 then start point will be included, if i = 1 first point omitted, if i < RESOLUTION_OF_t then end point omitted, if <= then endpoint included 
    for(int i = 0; i <= RESOLUTION_OF_T; i++) {
        t = ((double)1 / RESOLUTION_OF_T) * i;
        printf(" %lf %lf\n", cubic_bezier_x((double)0, (double)0, (double)1, (double)1, t),cubic_bezier_y((double)0, (double)1, (double)1, (double)0, t));
    }
*/
void decode_bezier(FILE* file, unsigned int RESOLUTION_OF_T, double start_x, double c1_x, double c2_x, double end_x, double start_y, double c1_y, double c2_y, double end_y) {
    double t = 0;
    /* Usage of cubic bezier functions, if int i = 0 then start point will be included, if i = 1 first point omitted, if i < RESOLUTION_OF_t then end point omitted, if <= then endpoint included */
    for(int i = 1; i <= RESOLUTION_OF_T; i++) { // Substitue the t value into the bezier function and print the result to a file
        t = ((double)1 / RESOLUTION_OF_T) * i;
        double x = round(cubic_bezier_x(start_x, c1_x, c2_x, end_x, t)); // Rounds the results to the nearest integer
        double y = round(cubic_bezier_y(start_y, c1_y, c2_y, end_y, t)); // Rounds the results to the nearest integer
        move_to_point(file, x, y);
    }
}

/* Function to check if the passed character is a number or a dot or not */
int is_num(char ch) {
    if(((ch > 47) && (ch< 58)) || (ch == 46)) //If ascii code of the char corresponds to a number or dot return 0
        return 0;
    else
        return 1; // Didn't find a number or dot
}

/* Function to check if the passed character is a char or not */
int is_char(char ch) {
    if(((ch > 64) && (ch < 91)) || ((ch > 96) && (ch < 123))) //If ascii code of the char corresponds to a upper or lower case letter return 0
        return 0;
    else
        return 1; // Didn't find a char
}

find_return find_next(char* str, char* next_return, int cursor, unsigned int length) { //Function to find the next instruction in the svg file
    find_return temp = find_return_init(); //Initialize the struct with 0s
    int temp_cursor = cursor; //Create a temp cursor to keep track of the cursor

    if(str[cursor] == 32) { //If the current character is a space then increment the cursor
        //printf("Current character is a space\n");
        cursor++;
    }
    if(!is_num(str[cursor])) { //Current character is a num
        //printf("Current character is a num\n");
        temp.result = NUM;
    }
    if(!is_char(str[cursor])) { //Current character is a char
        //printf("Current character is a char\n");
        temp.result = CHAR;
    }

    if(temp.result == ERR) //If the current character is not a num or char then return error
        return temp;
    else if(temp.result == NUM) { //If the current character is a num then go ahead with the operations
        while(!is_num(str[cursor]) && ((cursor < length) && (temp.increment < 10))) { //While the current character is a num and the increment is less than 10 and the cursor is less length of the file
            next_return[temp.increment] = str[cursor];
            temp.increment++;
            cursor++;
        }
        if(str[temp_cursor] == 32) { //This is to handle a bug with spaces
            //printf("Current character is a space\n");
            temp.increment++;
        }
        return temp;
    }
    else if(temp.result == CHAR) { //If the current character is a char then go ahead with the operations
        while(!is_char(str[cursor]) && ((cursor < length) && (temp.increment < 10))) { //While the current character is a char and the increment is less than 10 and the cursor is less length of the file
            next_return[temp.increment] = str[cursor];
            temp.increment++;
            cursor++;
        }
        if(str[temp_cursor] == 32) { //This is to handle a bug with spaces
            //printf("Current character is a space\n");
            temp.increment++;
        }
        return temp;
    }
}
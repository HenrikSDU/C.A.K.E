#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/decoding_functions.h"

/* Defines the amount of steps that 't' will have, quasi how many little lines (RESOLUTION_OF_T - 1 number of little lines if I'm correct) will make up a curve */
#define RESOLUTION_OF_T 10

/* Global variables to keep track of current position in the coordinate system and in the string  */
unsigned int current_x = 0, current_y = 0, cursor = 0, prev_cursor;



/* The idea of this is to use this enum as a more descriptive flag because it will have the d variant as a value when it detects the secon = sign */
enum path_line {
    null,
    stroke,
    _d,
};


int main() {
    /* fp stands for file pointer, and is of type file(FILE) pointer(*) */
    FILE* fp = fopen("test14.svg", "r"); // fopen will get the address of a file, and open it in a mode, here read
    FILE* gcode = fopen("test14.gcode", "w");

    unsigned int width, height;
    enum path_line obj = null;
    point_t current_pos = init_point();

    /* Determining the lenght of a file, not sure if this is the best way but it works */
    fseek(fp, 0, SEEK_END); // Putting cursor at the end
    int length = ftell(fp); // Getting cursor location (distance from the start of the file)
    printf("Length of file in bytes: %d\n", length);

    fseek(fp, 0, SEEK_SET);
    char* svg = malloc(length * sizeof(char) + 1);
    for(int i = 0; i < length; i++) {
        svg[i] = getc(fp);
    }
    svg[length] = 0;
    printf("%s\n", svg);


    cursor = str_find_width(svg, cursor, length);
    {
        int count = 0;
        while(svg[cursor + count] != 34) {
            count++;
        }
        char* width_temp = malloc((count + 1) * sizeof(char));
        for(int k = 0; k < count; k++) {
            width_temp[k] = svg[cursor + k];
        }
        width_temp[count] = 0;
        width = atoi(width_temp);
        free(width_temp);
    }

    cursor = str_find_height(svg, 0, length);
    {
        int count = 0;
        while(svg[cursor + count] != 34) {
            count++;
        }
        char* height_temp = malloc((count + 1) * sizeof(char));
        for(int k = 0; k < count; k++) {
            height_temp[k] = svg[cursor + k];
        }
        height_temp[count] = 0;
        height = atoi(height_temp);
        free(height_temp);
    }
    printf("Width: %d, Height: %d\n", width, height);

    // If more colors or layers are wanted on the same picture the stroke property could be useful and should be checked before the coordinates

    /* Temporary file for easier conversion */
    FILE* temp_d = fopen("temp_d.txt", "w");

    cursor = 0;
    while(svg[cursor] != 0) {
        cursor = str_find_d(svg, cursor, length); // Will return cursor location where it finds " "=d " or if it doesn`t then the length that was passed in as an argument
        printf("Cursor: %d\n", cursor);
        if(cursor == length) // If cursor is at the end then terminate the loop
            break;

        for(int i = cursor; i < length; i++) {
            // Bad practice but many if statements to decide how to print the current char, new lines chars and numbers matter
            if((svg[i] == 'M' || svg[i] == 'C' || svg[i] == 'L') && i == cursor) { // If letter and first char then no new line in the beginning
                fprintf(temp_d, "%c" , svg[i]);
                fprintf(temp_d, "\n");
            }
            else if((svg[i] == 'M' || svg[i] == 'C' || svg[i] == 'L') && svg[i - 1] < 65) { // If previous character was a number start in new line
                fprintf(temp_d, "\n");
                fprintf(temp_d, "%c" , svg[i]);
                fprintf(temp_d, "\n");
            }
            else if((svg[i] > 47 && svg[i] < 58) || svg[i] == 46) { // If number or . then write those in one line
                fprintf(temp_d, "%c" , svg[i]);
            }
            else if(svg[i] == 32) { // Putting \n instead of space
                fprintf(temp_d, "\n");
            }
            if(svg[i] == 34) // " is the terminating character for the d variable
                break;
        }
        fprintf(temp_d, "\n\n\n"); // Separation between objects
    }

    // I can do this but it`s quite pointless at this time
    fprintf(stderr, "gddf\n");
    fprintf(stdout, "gddf\n");
    printf("%c%c\n", 219, 219);
    printf("%c%c\n", 220, 177);
    printf("%c%c\n", 192, 193);


    printf("Bezier-curve coordinates using RESOLUTION_OF_T:\n");
    double t = 0;
    /* Usage of cubic bezier functions, if int i = 0 then start point will be included, if i = 1 first point omitted, if i < RESOLUTION_OF_t then end point omitted, if <= then endpoint included */
    for(int i = 0; i <= RESOLUTION_OF_T; i++) {
        t = ((double)1 / RESOLUTION_OF_T) * i;
        //printf("T: %lf ", t);
        printf(" %lf %lf\n", cubic_bezier_x((double)0, (double)0, (double)1, (double)1, t),cubic_bezier_y((double)0, (double)1, (double)1, (double)0, t));
    }



    printf("\nThe remaining characters\n");
    while(cursor < length) {
        printf("%c", svg[cursor]);
        cursor++;
    }

    fclose(fp);
    free(svg);
    //free(d);

    return 0;
}

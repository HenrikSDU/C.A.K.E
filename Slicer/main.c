#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/decoding_functions.h"
//#include "include/instructions.h"

/* Defines the amount of steps that 't' will have, quasi how many little lines (RESOLUTION_OF_T - 1 number of little lines if I'm correct) will make up a curve */
#define RESOLUTION_OF_T 30

int main() {
    /* fp stands for file pointer, and is of type file(FILE) pointer(*) */
    FILE* fp = fopen("test16.svg", "r"); // fopen will get the address of a file, and open it in a mode, here read
    FILE* gcode = fopen("test16.cake", "w");

    /* Variables to keep track of current position in the coordinate system and in the string  */
    unsigned int cursor = 0;
    unsigned int width, height;
    point_t current_pos = init_point();
    point_t prev_pos = init_point();
    point_t control1 = init_point();
    point_t control2 = init_point();
    point_t end_pos = init_point();
    command_t  command;

    /* Determining the lenght of a file, not sure if this is the best way but it works */
    fseek(fp, 0, SEEK_END); // Putting cursor at the end
    int length = ftell(fp); // Getting cursor location (distance from the start of the file)
    printf("Length of file in bytes: %d\n", length);

    fseek(fp, 0, SEEK_SET);
    char* svg = malloc(length * sizeof(char));
    for(int i = 0; i < length; i++) {
        svg[i] = getc(fp);
    }
    svg[length - 1] = 0; // Setting the last char to 0 to terminate the string
    
    /* Printing the string */
    //printf("%s\n", svg);
    fclose(fp);


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
        //printf("Cursor: %d\n", cursor);
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
        fprintf(temp_d, "\n\n"); // Separation between objects
    }
    fclose(temp_d);

    /* Converting the temporary file to gcode */
    FILE* convert = fopen("temp_d.txt", "r");
    if(convert == NULL) {
        fprintf(stderr, "Failed to open file");
        return 1;
    }

    unsigned int point_counter = 0;
    char read_line[10];
    for(int i = 0; i < 10; i++) {
        read_line[i] = 0;
    }
    do {
        printf("%s", read_line);
        if(read_line[0] == 'M') {
            command = M;
            point_counter = 0;
            continue;
        }
        else if(read_line[0] == 'C') {
            command = C;
            point_counter = 0;
            continue;
        }
        else if(read_line[0] == 'L') {
            command = L;
            point_counter = 0;
            printf("Prevpos %f %f\n", prev_pos.x, prev_pos.y);
            printf("Curpos %f %f\n", current_pos.x, current_pos.y);
            prev_pos = current_pos;
            continue;
        }
        else if(read_line[0] < 58 && read_line[0] > 47) {

            if(command  == M) {
                if(point_counter == 0) {
                    current_pos.x = atof(read_line);
                    printf("x: %f\n", current_pos.x);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = atof(read_line);
                    printf("y: %f\n", current_pos.y);
                    point_counter++;
                }
                extruder_up(gcode);
                move_to_point(gcode, current_pos.x, current_pos.y);
                extruder_down(gcode);
                prev_pos = current_pos;
            }
            else if(command == C) {
                if(point_counter == 0) {
                    control1.x = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    control1.y = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 2) {
                    control2.x = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 3) {
                    control2.y = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 4) {
                    end_pos.x = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 5) {
                    end_pos.y = atof(read_line);
                    point_counter++;
                }
                // now to use all this information to create a bezier curve
                decode_bezier(gcode, RESOLUTION_OF_T, prev_pos.x, control1.x, control2.x, end_pos.x, prev_pos.y, control1.y, control2.y, end_pos.y);
                prev_pos = end_pos;
            }
            else if(command == L) {
                if(point_counter == 0) {
                    current_pos.x = atof(read_line);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = atof(read_line);
                    point_counter++;
                }
                move_to_point(gcode, current_pos.x, current_pos.y);
                prev_pos = current_pos;
            }
        }
    } while(fgets(read_line, 20, convert) != NULL);

    /*
    bezier_t curve = init_bezier();
    curve.start_x = 0;
    curve.c1_x = 0;
    curve.c2_x = 10;
    curve.end_x = 10;
    curve.start_y = 0;
    curve.c1_y = 10;
    curve.c2_y = 10;
    curve.end_y = 0;
    decode_bezier(gcode, RESOLUTION_OF_T, curve.start_x, curve.c1_x, curve.c2_x, curve.end_x, curve.start_y, curve.c1_y, curve.c2_y, curve.end_y);
    */

    free(svg);
    fclose(gcode);
    fclose(temp_d);
    fclose(convert);

    return 0;
}

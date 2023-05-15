#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/decoding_functions.h"

/* Defines the amount of steps that 't' will have, quasi how many little lines (RESOLUTION_OF_T - 1 number of little lines if I'm correct) will make up a curve */
#define RESOLUTION_OF_T 6

int main(int argc, char **argv) {

    /* Error message and program termination if the user doesn't input the correct amount of arguments i.e. the executable and the path of the file to be sliced */
    if(argc != 3) {
        printf("Usage: .%cSlicer.exe <input file> <output file>\n", 92);
        return 1;
    }

    // fp stands for file pointer, and is of type file(FILE) pointer(*)
    FILE* fp = fopen(argv[1], "r"); // fopen will get the address of a file, and open it in a mode, here read
    FILE* cake = fopen(argv[2], "w"); // Opening the output file in write mode (overwrites the file if it already exists)

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
    unsigned length = ftell(fp); // Getting cursor location (distance from the start of the file)
    printf("Length of file in bytes: %d\n", length);

    fseek(fp, 0, SEEK_SET); // Putting cursor back at the start
    char* svg = malloc(length * sizeof(char)); // Dynamically allocating memory for the string
    for(int i = 0; i < length; i++) { // Reading the file into the string (basically putting it into memory)
        svg[i] = getc(fp);
    }
    svg[length - 1] = 0; // Setting the last char to 0 to terminate the string
    
    // Closing the file to free up memory
    fclose(fp);


    //maybe we can give here an example of an svg-file

    // Finding the width of the picture
    cursor = str_find_width(svg, cursor, length);
    {
        int count = 0;
        while(svg[cursor + count] != 34) { // Counts until the next " is found
            count++;
        }

        char* width_temp = malloc((count + 1) * sizeof(char)); // Dynamically allocating memory for the string
        for(int k = 0; k < count; k++) { // Reads the width into the string
            width_temp[k] = svg[cursor + k];
        }
        width_temp[count] = 0;
        width = atoi(width_temp); // atoi converts a string to an integer
        free(width_temp);
    }

    // Finding the height of the picture
    cursor = str_find_height(svg, 0, length);
    {
        int count = 0;
        while(svg[cursor + count] != 34) { // Counts until the next " is found
            count++;
        }
        char* height_temp = malloc((count + 1) * sizeof(char)); // Dynamically allocating memory for the string
        for(int k = 0; k < count; k++) { // Reads the height into the string
            height_temp[k] = svg[cursor + k];
        }
        height_temp[count] = 0;
        height = atoi(height_temp); // atoi converts a string to an integer
        free(height_temp);
    }
    printf("Width: %d, Height: %d\n", width, height);

    // If more colors or layers are wanted on the same picture the stroke property could be useful and should be checked before the coordinates


    //maybe explain the purpose of find_return here

    // Variable used for decoding the d property
    find_return find_ret = find_return_init(); // Custom struct defined in include/decoding_functions.h
    char temp[10]; // Temporary string used to store a letter or a number, fixed size due to the fact that the longest number is less than 10 digits long
    memset(temp, 0, 10); // Initializing the string to 0, so no undefined behaviour occurs
    unsigned int point_counter = 0; // Used to keep track of how many coordinates have been read, 1 for normal operations, 4 for bezier curves
    int length_of_d = 0; // Used to keep track of the length of the d property
    int inside_d = 0;   // Variable used to keep track of whether the "cursor"(which byte we are reading inside the d property) is inside the d property or not

    // Finding the length of the d property
    cursor = str_find_d(svg, 0, length);
    while(svg[cursor + length_of_d] != 34) { // Counts until the next " is found
        length_of_d++;
    }

    do {
        find_ret = find_next(svg, temp, cursor, length);
        //printf("Result of find_*: %d\n", find_ret.result);
        cursor += find_ret.increment;


        //why no switch statement xD?
        if(temp[0] == 'M') { // Checking if the command is a move command
            command = M; // Setting the command to move
            point_counter = 0;
            continue; // Skipping the rest of the loop
        }
        else if(temp[0] == 'C') { // Checking if the command is a bezier curve command
            command = C; // Setting the command to bezier curve
            point_counter = 0;
            continue;
        }
        else if(temp[0] == 'L') { // Checking if the command is a "line to" command
            command = L; // Setting the command to line to
            point_counter = 0;
            prev_pos = current_pos; // Setting the previous position to the current position
            continue;
        }
        else if(find_ret.result == NUM) { // Checking if the result of the find_next function is a number

            if(command  == M) { // If the command is a move command
                if(point_counter == 0) { // These if statements are used to determine which coordinate is being read ( x or y )
                    current_pos.x = atof(temp); // atof converts a string to a float
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = atof(temp); // atof converts a string to a float
                    point_counter++;
                }
                extruder_up(cake); // Lifting the extruder
                move_to_point(cake, current_pos.x, current_pos.y); // Moving to the point
                extruder_down(cake); // Lowering the extruder
                prev_pos = current_pos; 
            }
            else if(command == C) { // If the command is a bezier curve command
                if(point_counter == 0) {
                    control1.x = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    control1.y = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 2) {
                    control2.x = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 3) {
                    control2.y = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 4) {
                    end_pos.x = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 5) {
                    end_pos.y = atof(temp);
                    point_counter++;
                }
                // Now to use all this information to create a bezier curve
                decode_bezier(cake, RESOLUTION_OF_T, prev_pos.x, control1.x, control2.x, end_pos.x, prev_pos.y, control1.y, control2.y, end_pos.y);
                prev_pos = end_pos;
            }
            else if(command == L) { // If the command is a "line to" command
                if(point_counter == 0) {
                    current_pos.x = atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = atof(temp);
                    point_counter++;
                }
                move_to_point(cake, current_pos.x, current_pos.y); // Moving to the point
                prev_pos = current_pos;
            }
        }

        //Resetting variables that are used in the loop
        find_ret = find_return_init();
        memset(temp, 0, 10); // Setting the string to 0

        //Incerementing inside_d to go through d
        inside_d++;
    } while(inside_d < length_of_d);

    // Freeing up memory by closing files and freeing malloce-d memory (dynamically(at runtime) allocated memory)
    free(svg);
    fclose(cake);

    return 0;
}

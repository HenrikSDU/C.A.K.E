#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/decoding_functions.h"


int main(int argc, char **argv) {

    /* Error message and program termination if the user doesn't input the correct amount of arguments i.e. the executable and the path of the file to be sliced */
    if(argc != 4) {
        printf("Usage: .%cSlicer.exe <how many lines should make up a curve> <input file> <output file>\n", 92);
        return 1;
    }

    // fp stands for file pointer, and is of type file(FILE) pointer(*)
    FILE* fp = fopen(argv[2], "r"); // fopen will get the address of a file, and open it in a mode, here read
    FILE* cake = fopen(argv[3], "w"); // Opening the output file in write mode (overwrites the file if it already exists)

    /* Variables to keep track of current position in the coordinate system and in the string  */
    unsigned int cursor = 0;
    unsigned int width, height;
    point_t current_pos = init_point();
    point_t prev_pos = init_point();
    point_t control1 = init_point();
    point_t control2 = init_point();
    point_t end_pos = init_point();
    command_t  command;

    /* Defines the amount of steps that 't' will have, quasi how many little lines (RESOLUTION_OF_T - 1 number of little lines if I'm correct) will make up a curve */
    unsigned char resolution_of_t = atoi(argv[1]);

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

    /* Scaling variables, since our canvas is only 200x200 if a picture is bigger it has to be scaled down */
    float scale_x = 200 / (float)width;
    float scale_y = 200 / (float)height;
    printf("Scale_x: %f, Scale_y: %f\n", scale_x, scale_y);

    // If more colors or layers are wanted on the same picture the stroke property could be useful and should be checked before the coordinates

    // Variable used for decoding the d property
    find_return find_ret = find_return_init(); // Custom struct defined in include/decoding_functions.h
    char temp[10]; // Temporary string used to store a letter or a number, fixed size due to the fact that the longest number is less than 10 digits long
    memset(temp, 0, 10); // Initializing the string to 0, so no undefined behaviour occurs
    unsigned int point_counter = 0; // Used to keep track of how many coordinates have been read, 1 for normal operations, 4 for bezier curves
    int length_of_d = 0; // Used to keep track of the length of the d property
    int inside_d = -1;   // Variable used to keep track of whether the "cursor"(which byte we are reading inside the d property) is inside the d property or not

    // Finding the length of the d property
    cursor = str_find_d(svg, 0, length);
    while(svg[cursor + length_of_d] != 34) { // Counts until the next " is found
        //printf("cursor + length_of_d %d\n", cursor + length_of_d);
        length_of_d++;
    }
    printf("Length of d: %d and the char there %c Cursor %d\n", length_of_d, svg[cursor + length_of_d], cursor);

    do {
        //Resetting variables that are used in the loop
        find_ret = find_return_init();
        memset(temp, 0, 10); // Setting the string to 0

        //Incerementing inside_d to go through d
        inside_d++;

        find_ret = find_next(svg, temp, cursor, length);
        //printf("Result of find_*: %d\n", find_ret.result);
        cursor += find_ret.increment;

        printf("Temp: %s | Inside d %d\n", temp, inside_d);


        //why no switch statement xD? i like my if statemnts
        if(temp[0] == 'M') { // Checking if the command is a move command
            command = M; // Setting the command to move
            point_counter = 0;
            continue;
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
                    current_pos.x = scale_x * atof(temp); // atof converts a string to a float
                    printf("Current_pos.x: %f\n", current_pos.x);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = scale_y * atof(temp); // atof converts a string to a float
                    printf("Current_pos.y: %f\n", current_pos.y);
                    point_counter++;
                }
                extruder_up(cake); // Lifting the extruder
                move_to_point(cake, (int)round(current_pos.x), (int)round(current_pos.y)); // Moving to the point
                extruder_down(cake); // Lowering the extruder
                prev_pos = current_pos; 
            }
            else if(command == C) { // If the command is a bezier curve command
                if(point_counter == 0) {
                    control1.x = scale_x * atof(temp);
                    printf("Control1.x: %f\n", control1.x);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    control1.y = scale_y * atof(temp);
                    printf("Control1.y: %f\n", control1.y);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 2) {
                    control2.x = scale_x * atof(temp);
                    printf("Control2.x: %f\n", control2.x);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 3) {
                    control2.y = scale_y * atof(temp);
                    printf("Control2.y: %f\n", control2.y);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 4) {
                    end_pos.x = scale_x * atof(temp);
                    printf("End_pos.x: %f\n", end_pos.x);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 5) {
                    end_pos.y = scale_y * atof(temp);
                    printf("End_pos.y: %f\n", end_pos.y);
                    point_counter++;
                }
                // Now to use all this information to create a bezier curve
                decode_bezier(cake, resolution_of_t, prev_pos.x, control1.x, control2.x, end_pos.x, prev_pos.y, control1.y, control2.y, end_pos.y);
                prev_pos = end_pos;
            }
            else if(command == L) { // If the command is a "line to" command
                if(point_counter == 0) {
                    current_pos.x = scale_x * atof(temp);
                    point_counter++;
                    continue;
                }
                else if(point_counter == 1) {
                    current_pos.y = scale_y * atof(temp);
                    point_counter++;
                }
                move_to_point(cake, (int)(round(current_pos.x)), (int)(round(current_pos.y))); // Moving to the point
                prev_pos = current_pos;
            }
        }

    } while(inside_d < length_of_d);

    // Freeing up memory by closing files and freeing malloce-d memory (dynamically(at runtime) allocated memory)
    free(svg);
    fclose(cake);

    FILE* length_print = fopen(argv[3], "r");
    fseek(length_print, 0, SEEK_END);
    int length_of_output = ftell(length_print);
    printf("Length of output: %d\n", length_of_output);
    fclose(length_print);

    return 0;
}

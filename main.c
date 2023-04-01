#include <stdio.h>
#include <stdlib.h>

/* This is just something the tutorial said */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h" /* This means to include the header file located in the stb_image folder in the folder of the executable(or source code i don't know)*/
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "stb_image/stb_image_write.h"

/* Functions made by me, in different files to clean up the main c file */
#include "include/cursor_pos.h" //Custom function, would move the cursor in the terminal
#include "include/pixel_functions.h"  
#include "include/instructions.h" //All functions used to create the gcode file
#include "include/matrix_display.h"  

#define DIMENSION 10 /* Define the number of rows and columns of the PNG here, for now has to be defined in every header where it's used */

int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("Invalid usage.\nTry: *executable name* *name of picture*");
        exit(1);
    }
    /* By printing these you can see that the first argument is the executable itself and a second is what you input */
    //printf("%s\n", argv[0]);
    //printf("Second argument: %s, Length: %d\n", argv[1], sizeof(argv[1]) / sizeof(char));
    printf("Second argument: %s, Length: %d\n", argv[1], strlen(argv[1]));

    /* File stuff */
    char* filename = (char*)malloc(strlen(argv[1])); // Allocating memory for dynamically defined array
    for(int i = 0; i < strlen(argv[1]) + 1; i++) { // Copying the value of input argument to different array
        filename[i] = argv[1][i];
        printf("%d ", argv[1][i]);
    }

    //Debugging stuff
    printf("\n");
    int name_length = strlen(argv[1]);
    printf("%s\n", filename);

    /* Changing png to txt */
    for(int i = 0; i < name_length; i++) {
        if(filename[i] == 46) {
            filename[i+1] = 't';
            filename[i+2] = 'x';
            filename[i+3] = 't';
        }
    }

    //Debugging stuff
    printf("%s\n", filename);
    for(int i = 0; i < name_length + 1; i++) {
        printf("%d ", filename[i]);
    }
    printf("\n");

    FILE* file = fopen(filename, "w"); // Make a filepointer and assign it a value with fopen()
    // first argument is the name of the file, second is the mode of opening, this case is w for write, creating a file 

    //Test
    move_to_point(file, 10, 10);

    char* name = argv[1]; /* Change this to change the picture that will be processed */
    //char* name = "test4.png"; /* Change this to change the picture that will be processed */
    unsigned int width, height, channels; /* These will get values after the stbi_load() function */
    unsigned char* img; /* Essentially just an array of chars */

    /* Function to load the values of the pixels into an array */
    img = stbi_load(name, &width, &height, &channels, 0);

    /* Little bit of error handling */
    if(img == 0) {
        printf("Failed to load image.\n");
        exit(1);
    }
    /* Just some cool info about the image */
    printf("Loaded image, width: %dpx, height: %dpx, channels: %d\n", width, height, channels);
    printf("The RGB pixel values of the image:\n\n");


    /* Not dynamically allocating memory, less flexibility, more memory safety */
    unsigned char img_matrix[DIMENSION][DIMENSION];
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            img_matrix[i][j] = img[((i * DIMENSION) + j) * channels];
        }
    }
    /* Displaying the img_matrix */
    printf("IMG_MATRIX\n");
    matrix_display(DIMENSION, img_matrix);
    printf("\n");

    /* Average matrix */
    unsigned char avg_matrix[DIMENSION][DIMENSION];
    /* Filling the matrix with values*/
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            avg_matrix[i][j] = 255;
        }
    }

    unsigned char del_matrix[DIMENSION][DIMENSION];
    /* Filling the matrix with values*/
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            del_matrix[i][j] = img_matrix[i][j];
        }
    }

    /* Displaying the img_matrix but differently */
    printf("DEL_matrix\n");
    matrix_display(DIMENSION, del_matrix);
    printf("\n");
    printf("DEL_matrix\n");
    matrix_display(DIMENSION, del_matrix);

    printf("\n----------------------------------------\n");

    /* Deleting a column from x1 to x2 in column y */
    printf("DEL_matrix\n");
    matrix_display(DIMENSION, del_matrix);
    printf("\n");
    printf("Pixel_check_up + 1: %d, Pixel_check_down: %d\n", pixel_check_up(0, 0, del_matrix) + 1, pixel_check_down(0, 0, del_matrix));
    { // Extra indentation bc local_i_top should not exist outside the for loop, function to set the max of i didn't work, this is a solution
        int local_i_top = pixel_check_down(0, 0, del_matrix);
        for(int i = pixel_check_up(0, 0, del_matrix) + 1; i < local_i_top; i++) {
            del_matrix[i][0] = 255;
        }
    }
    printf("DEL_matrix\n");
    matrix_display(DIMENSION, del_matrix);
    
    //Testing functions

    for(int i =  0; i < 10; i++) {
        for(int y = 0; y < 10; y++) {
            avg_matrix[average_no_comment(pixel_check_up(i, y, img_matrix), pixel_check_down(i, y, img_matrix))][y] = 0;

            /* Debugging printfs */
            /*
            printf("Pixel num: %d\n", i * DIMENSION + y);
            printf("testing every pixel average: %d\n\n", average(pixel_check_up(i, y, img_matrix), pixel_check_down(i, y, img_matrix)));
            */
        }
    }
    printf("\n");
 
    /* Displaying the average matrix */
    printf("AVG_MATRIX\n");
    matrix_display(DIMENSION, avg_matrix);
    printf("\n");

    printf("Pixel check forward: %d\n", pixel_check_forward(4, 2, img_matrix));
    printf("Pixel check backwards: %d\n", pixel_check_back(4, 2, img_matrix));
    printf("\n");

    /* ******************************************************************************** */
    /* The main procedure with all the stuff */
    // New matrix, will be edited along the way
    unsigned char temp_matrix[DIMENSION][DIMENSION];
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            temp_matrix[i][j] = img_matrix[i][j]; // Copying the matrix because this will be wiped as it is being processed
        }
    }
    // Wiping avg_matrix, this will be handled better, but for testing purposes the previous operations help
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            avg_matrix[i][j] = 255;
        }
    }

    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            unsigned char line_end_flag = 0;

            int x = i;
            int y = j;

            int x_lim = DIMENSION;
            int y_lim = DIMENSION;

            if(temp_matrix[x][y] == 0) {
                extruder_down(file);
            }


                int x_top = pixel_check_up(x, y, temp_matrix);
                int x_down = pixel_check_down(x, y, temp_matrix);
                int avg = average_no_comment(x_top, x_down);

                // Filter for edge-cases
                if(y > 0 || y < 9) { // Not on the edge
                    if(pixel_check_back(x, y, temp_matrix) != -2) {
                    
                    }
                    else if(pixel_check_forward(x, y, temp_matrix) != -2) {

                    }
                    else {
                        line_end_flag = 1;
                    }
                }
                else if(y == 0) { // On the edge left side
                    if(pixel_check_forward(x, y, temp_matrix) != -2) {

                    }
                    else {
                        line_end_flag = 1;
                    }
                }
                else if(y == 9) { // On the edge right side
                    if(pixel_check_back(x, y, temp_matrix) != -2) {

                    }
                    else {
                        line_end_flag = 1;
                    } 
                }


                // Clearing the column so it won't be checked again
                for(int i = x_top + 1; i < x_down; i++) {
                    temp_matrix[i][y] = 255;
                }
            
        }
    }


    /* Closing file */
    fclose(file);

    /* VERY IMPORTANT! ALWAYS FREE IMAGE WITH stbi_image_free(), otherwise there'll be a memory leak */
    free(filename);
    stbi_image_free(img);
    return 0;
}

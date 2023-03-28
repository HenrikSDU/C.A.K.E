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
    move_along_line(file, 10, 10);

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
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            printf("%d ", img_matrix[i][j]);
        }
        printf("\n");
    }

    printf("\n----------------------------------------\n");
    //Currently useless
    //printf("Return value of pixel_surround: %d\n", pixel_surround_check(3, 1, img_matrix));

    //Testing functions
    /*
    printf("Return of pixel_check_up: % d\n", pixel_check_up(6, 2, img_matrix));
    printf("Return of pixel_check_down: % d\n", pixel_check_down(6, 2, img_matrix));
    printf("The midpoint of the current column: %d\n", average(pixel_check_up(6, 2, img_matrix), pixel_check_down(6, 2, img_matrix)));
    printf("ßßßßßßßßßß\n");
    printf("Return of pixel_check_up: % d\n", pixel_check_up(9, 0, img_matrix));
    printf("Return of pixel_check_down: % d\n", pixel_check_down(9, 0, img_matrix));
    printf("The midpoint of the current column: %d\n", average(pixel_check_up(9, 0, img_matrix), pixel_check_down(9, 0, img_matrix)));
    printf("ßßßßßßßßßß\n");
    printf("Return of pixel_check_up: % d\n", pixel_check_up(8, 0, img_matrix));
    printf("Return of pixel_check_down: % d\n", pixel_check_down(8, 0, img_matrix));
    printf("The midpoint of the current column: %d\n", average(pixel_check_up(8, 0, img_matrix), pixel_check_down(8, 0, img_matrix)));

    printf("ßßßßßßßßßß\n");
    printf("ßßßßßßßßßß\n");
    */
    
    for(int i =  0; i < 10; i++) {
        for(int y = 0; y < 10; y++) {
            printf("Pixel num: %d\n", i * DIMENSION + y);
            printf("testing every pixel average: %d\n", average(pixel_check_up(i, y, img_matrix), pixel_check_down(i, y, img_matrix)));
            printf("\n");
        }
    }
 
    /* Closing file */
    fclose(file);

    /* VERY IMPORTANT! ALWAYS FREE IMAGE WITH stbi_image_free(), otherwise there'll be a memory leak */
    free(filename);
    stbi_image_free(img);
    return 0;
}


/*
Line separator 2000

How it works in my head:
    First pixel
        Record position of current pixel if useful (not white (255))
        Write current pixel to white (not useful)
        Test every neighbouring pixel if they have a value other than white (255)
        If yes, proceed
        If no, end of line
    Next pixel
        Record position of current pixel if useful (not white (255))
        Write current pixel to white (not useful)
        Test every neighbouring pixel if they have a value other than white (255)
        If yes, proceed
        If no, end of line
*/




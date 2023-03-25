#include <stdio.h>
#include <stdlib.h>

/* This is just something the tutorial said */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h" /* This means to include the header file located in the stb_image folder in the folder of the executable(or source code i don't know)*/
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "stb_image/stb_image_write.h"

/* Functions made by me, in different files to clean up the main c file */
#include "include/cursor_pos.h" //Custom function, would move the cursor in the terminal
#include "include/pixel_surround_check.h" 

#define DIMENSION 10 /* Define the number of rows and columns of the PNG here */
#define file_FILE_LENGHT 12 /* Max length of the file name the instructions are saved in */

/* Instruction writing functions */
void move_along_line(FILE*, int, int);

/* Pixel evaluation functions */
int pixel_check_and_erase(int, int, unsigned char (*)[DIMENSION]);
int pixel_surround_check(int, int, unsigned char (*)[DIMENSION]);

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
    /* One way to display the pixel values, 3 8 bit numbers give the Red Green Blue values of one pixel */
    /*
    for(int i = 0; i < (width*height*channels); i = i + 3) {
        if(i % width == 0) 
            printf("\n");

        printf("[%d %d %d] ", img[i], img[i + 1], img[i + 2]);
    }
    */

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
    printf("Return value of pixel_surround: %d\n", pixel_surround_check(3, 1, img_matrix));


    /* Closing file */
    fclose(file);

    /* VERY IMPORTANT! ALWAYS FREE IMAGE WITH stbi_image_free(), otherwise there'll be a memory leak */
    free(filename);
    stbi_image_free(img);
    return 0;
}

void move_along_line(FILE* file, int x, int y) {
    fprintf(file, "X%dY%d", x, y);
}

/* Checks if pixel is useful, "erases" it if yes and returns 0 */
int pixel_check_and_erase(int x, int y, unsigned char (*img_matrix)[DIMENSION]) {
    if(img_matrix[x][y] != 255) {
        img_matrix[x][y] = 255;
        return 0;
    }
    else
        return 1;
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
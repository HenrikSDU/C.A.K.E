#include <stdio.h>
#include <stdlib.h>
/* This is just something the tutorial said */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h" /* This means to include the header file located in the stb_image folder in the folder of the executable(or source code i don't know)*/
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "stb_image/stb_image_write.h"

int main() {
    unsigned char* name = "test4.png"; /* Change this to change the picture that will be processed */
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

    /* One way to display the pixel values, 3 8 bit numbers give the Red Green Blue values of one pixel */
    for(int i = 0; i < (width*height*channels); i = i + 3) {
        if(i % width == 0) 
            printf("\n");

        printf("[%d %d %d] ", img[i], img[i + 1], img[i + 2]);
    }

    /* VERY IMPORTANT! ALWAYS FREE IMAGE WITH stbi_image_free(), otherwise there'll be a memory leak */
    stbi_image_free(img);
    return 0;
}


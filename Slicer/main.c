#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/decoding_functions.h"

/* Defines the amount of steps that 't' will have, quasi how many little lines (RESOLUTION_OF_T - 1 number of little lines if I'm correct) will make up a curve */
#define RESOLUTION_OF_T 20

#define START_OF_SVG 39 // The first line of the file is not useful here therefore skipping over it
#define WIDTH_POS 51 // This is where the value of the width of the picture is stored
#define W_H_DELTA 13 // The difference between the end of the width value and the beginning of the height



int main() {
    /* fp stands for file pointer, and is of type file(FILE) pointer(*) */
    FILE* fp = fopen("test14.svg", "r"); // fopen will get the address of a file, and open it in a mode, here read

    unsigned int width, height;

    /* Determining the lenght of a file, not sure if this is the best way but it works */
    fseek(fp, 0, SEEK_END); // Putting cursor at the end
    int length = ftell(fp); // Getting cursor location (distance from the start of the file)
    printf("Length of file in bytes: %d\n", length);

    /* Small little function to determine the width and height indicated in the svg */
    {
        char str[4];
        fseek(fp, WIDTH_POS, SEEK_SET);
        for(int i = 0; i < 4; i++) {
            char temp_ch = getc(fp);
            printf("%d\n", temp_ch);
            if(temp_ch == 34) {
                break;
            }
            str[i] = temp_ch;
        }
        printf("Str: %s\n", str);
        width = atoi(str);
        printf("Width: %d\n", width);

        memset(str, 0, sizeof(str)); // Resetting the string, there was an issue with the strings but this fixed it

        while(getc(fp) != 34);

        for(int i = 0; i < 4; i++) {
            char temp_ch = getc(fp);
            printf("%d\n", temp_ch);
            if(temp_ch > 57 ||temp_ch < 48) {
                break;
            }
            str[i] = temp_ch;
        }
        printf("Str: %s\n", str);
        height = atoi(str);
        printf("Height: %d\n", height);
    }
    
    // Small testing for the str_shift_left function
    char test1[] = "asd";
    str_shift_left(test1, sizeof(test1), 'b');
    printf("Asd: %s\n %d %d %d %d", test1, test1[0], test1[1], test1[2], test1[3]);

   

    fclose(fp);

    return 0;
}

// i have to learn about pipes now if i dont want to have a temporary file ehhhhhhhhhh

/* All functions related to pixel checking, setting or clearing */
/* Error as a return value is mostly used for error messages */

#include <stdio.h>
#include <math.h>

#define DIMENSION 10 //Has to be manually set for every function that uses it, suboptimal, but I don't want to get into preprocessors yet
#define ERROR -2
#define SUCCESS 0
#define FAIL 1

/* Checks pixels around current pixel specified by x in y in the specified matrix 
   Every pixel that shares an edge or vertex with the current one
   Returns 0 if there are any non white(255), returns 1 if there aren't */
int pixel_surround_check(int x, int y, unsigned char (*img_matrix)[DIMENSION]) {
    int limit_x[2] = {-1, 2};
    int limit_y[2] = {-1, 2};;
    if(x == 0)
        limit_x[0] = 0, limit_x[1] = 2;
    if(x == 9)
        limit_x[0] = -1, limit_x[1] = 1;
    if(y == 0)
        limit_y[0] = 0, limit_x[1] = 2;
    if(y == 9)
        limit_x[0] = -1, limit_x[1] = 1;

    //Debug
    printf("limit_x: %d, %d\n", limit_x[0], limit_x[1]);
    printf("limit_y: %d, %d\n", limit_y[0], limit_y[1]);

    int current_pixel[9];
    int current_pixel_index = 0;
    
    for(int i = limit_x[0]; i < limit_x[1]; i++) {
        for(int j = limit_y[0]; j < limit_y[1]; j++) {
            printf("i = %d, j = %d\n", i, j);
            printf("Coords of pixel checked: x: %d, y: %d. Value of pixel: %d\n", x + i, y + i, img_matrix[x + i][y + j]);
            current_pixel[current_pixel_index] = img_matrix[x + i][y + j];
            current_pixel_index++;
        }
    }

    for(int i = 0; i < 9; i ++) {
        if(i != 4 && current_pixel[i] == 0)
            return 0;
    }
    return 1;
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

/* This checks pixels above until it finds one that isn't useful
   Returns the number of pixels from the current one where it found the first not useful one */
int pixel_check_up(int x, int y, unsigned char (*matrix)[DIMENSION]) {
    int x_up;

    
    if(matrix[x][y] == 255) // Error handling
        return ERROR;
    if(x == 0) {
        return -1;
    }
    for(x_up = x; x_up > -1; x_up--) {
        //printf("current check %d\n", matrix[x_up][y]);
        if(matrix[x_up][y] == 255) {
            return x_up;
        }
    }
    return -1;
}

/* Same as previous function just downwards, graphically down */
int pixel_check_down(int x, int y, unsigned char (*matrix)[DIMENSION]) {
    int x_down;

    if(matrix[x][y] == 255) // Error handling
        return ERROR;
    if(x == 9) {
        return DIMENSION;
    }

    for(x_down = x; x_down < DIMENSION; x_down++) {
        //printf("current check %d\n", matrix[x_down][y]);
        if(matrix[x_down][y] == 255) {
            return x_down;
        }
    }
    return DIMENSION;
}

/* Deletes from x_top until x_bottom in the column y */
void pixel_column_delete(int x_top, int x_down, int y, unsigned char matrix[DIMENSION][DIMENSION]) {
    for(int i = x_top; i >= x_down; i--) {
        matrix[i][y] = 255;
    }
}

/* I don't think I have to explain this */
int average(int m, int n) {
   printf("check up value inside average: %d\n", m);
   printf("check down value inside average: %d\n", n);
    float average = (m + n) / 2.0;
    return (int)round(average); // For now just typecasting to int, but I may add rounding
}

/* Average funcion without comments for now */
int average_no_comment(int m, int n) {
    float average =((float)m + (float)n) / 2.0;
    return (int)round(average); // For now just typecasting to int, but I may add rounding
}

/* Checks the column of pixels from x_up to x_down in the next column, returns ROW coordinate of the first non-white point from top to bottom */
int pixel_check_forward(int x, int y, unsigned char (*matrix)[DIMENSION]) {
    int x_up = pixel_check_up(x, y, matrix);
    int x_down = pixel_check_down(x, y, matrix);

    if(x_up == ERROR || x_down == ERROR) { // Error handling and propagation
        return ERROR;
    }
    for(int i = x_up; i <= x_down; i++) {
        if(matrix[i][y+1] == 0) {
            return i;
        }
    }
    return ERROR;
}

/* Checks the column of pixels from x_up to x_down in the previous column, returns ROW coordinate of the first non-white point from top to bottom */
int pixel_check_back(int x, int y, unsigned char (*matrix)[DIMENSION]) {
    int x_up = pixel_check_up(x, y, matrix);
    int x_down = pixel_check_down(x, y, matrix);

    if(x_up == ERROR || x_down == ERROR) { // Error handling and propagation
        return ERROR;
    }
    for(int i = x_up; i <= x_down; i++) {
        if(matrix[i][y-1] == 0) {
            return i;
        }
    }
    return ERROR;
}

/* Goes through the matrix, returns ROW number of first point with 0 as its value */
int pixel_scan_x(unsigned char (*matrix)[DIMENSION]) {
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            if(matrix[i][j] == 0) 
                return i;
        }
    }
    return ERROR;
}

/* Goes through the matrix, returns COLUMN number of first point with 0 as its value */
int pixel_scan_y(unsigned char (*matrix)[DIMENSION]) {
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            if(matrix[i][j] == 0) 
                return j;
        }
    }
    return ERROR;
}

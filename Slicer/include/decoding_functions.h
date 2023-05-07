#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "instructions.h"

typedef struct {
    double x;
    double y;
} point_t;

point_t init_point(void) {
    point_t point;
    point.x = 0.0;
    point.y = 0.0;
    return point;
}

typedef struct {
    double start_x;
    double start_y;
    double c1_x;
    double c1_y;
    double c2_x;
    double c2_y;
    double end_x;
    double end_y;
} bezier_t;

bezier_t init_bezier() {
    bezier_t bezier;
    bezier.start_x = 0;
    bezier.start_y = 0;
    bezier.c1_x = 0;
    bezier.c1_y = 0;
    bezier.c2_x = 0;
    bezier.c2_y = 0;
    bezier.end_x = 0;
    bezier.end_y = 0;
    return bezier;   
}

typedef enum {
    M,
    C,
    L,
} command_t;

/* Bézier-curve deciphering functions */
double cubic_bezier_x(double start_x, double c1_x, double c2_x, double end_x, double t) {
    double ratio = (double)1 - t;
    double x = start_x * pow(ratio, (double)3) + c1_x * (double)3*t * pow(ratio, (double)2) + c2_x * (double)3*pow(t, (double)2) * ratio + end_x * pow(t, (double)3);
    return x;
}

double cubic_bezier_y(double start_y, double c1_y, double c2_y, double end_y, double t) {
    double ratio = (double)1 - t;
    double y = start_y * pow(ratio, (double)3) + c1_y * (double)3*t * pow(ratio, (double)2) + c2_y * (double)3*pow(t, (double)2) * ratio + end_y * pow(t, (double)3);
    return y;
}


/* Small example for the str_shift_left function
    char test1[] = "asd";
    str_shift_left(test1, sizeof(test1), 'b');
    printf("Asd: %s\n", test1);
    this will print "bas" */ 
void str_shift_left(char* str, int length, char next) {
    char* temp = malloc((length - 1) * sizeof(char));
    for(int i = 1; i < length - 1 + 1; i++) {
        temp[i] = str[i - 1]; // temp = _as
    }
    temp[0] = next;
    for(int i = 0; i < length - 1; i++) {
        str[i] = temp[i];
    }

    free(temp);
}

unsigned int str_find_d(char* str, unsigned int cursor, unsigned int length) {
    char d[4]; //The string we want to find, has to be inputted backwards except for the NULL char
    d[0] = 34; // "
    d[1] = 61; // =
    d[2] = 100; // d
    d[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) {
        str_shift_left(temp, sizeof(temp), str[i]);
        //printf("%s\n", temp);
        match = 0;
        for(int j = 0; j < sizeof(d); j++) {
            //printf("the compared chars: %c, %c\n", d[j], temp[j]);
            if(d[j] == temp[j])
                match++;
        }
        //printf("match is: %d\n", match);
        if(match == 4)
            return i + 1; // Returns the current "position" basically where the specified string ends
        }
    return length; //Error return is yet to be decided
}

unsigned int str_find_width(char* str, unsigned int cursor, unsigned int length) {
    char width[4]; //The string we want to finwidth, has to be inputtewidth backwarwidths except for the NULL char
    width[0] = 34; // "
    width[1] = 61; // =
    width[2] = 104; // h
    width[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) {
        str_shift_left(temp, sizeof(temp), str[i]);
        match = 0;
        for(int j = 0; j < sizeof(width); j++) {
            if(width[j] == temp[j])
                match++;
        }
        if(match == 4)
            return i + 1; // Returns the current "position" basically where the specifiewidth string enwidths
        }
    return length; //Error return is the lenght of the file meaning it didn't find the width
}

unsigned int str_find_height(char* str, unsigned int cursor, unsigned int length) {
    char height[4]; //The string we want to finwidth, has to be inputtewidth backwarwidths except for the NULL char
    height[0] = 34; // "
    height[1] = 61; // =
    height[2] = 116; // t
    height[3] = 0; // NULL
    char temp[4];
    unsigned int match = 0; //Counts how many matches there are between the strings

    for(int i = cursor; i < length; i++) {
        str_shift_left(temp, sizeof(temp), str[i]);
        match = 0;
        for(int j = 0; j < sizeof(height); j++) {
            if(height[j] == temp[j])
                match++;
        }
        if(match == 4)
            return i + 1; // Returns the current "position" basically where the specifiewidth string enwidths
        }
    return length; //Error return is the lenght of the file meaning it didn't find the height
}

point_t get_coords(char* str, unsigned int* cursor) {
    point_t point;
    char coord[10]; // 10 for now as that should be enough for every number however more logic can be added to set a resolution for floats
    int count = 0;
    while(str[*cursor] > 47 || str[*cursor] < 58) {
        coord[count] = str[*cursor];
        count++;
        *cursor++;
    }
    point.x = atof(coord);

    memset(coord, 0, sizeof(char) * sizeof(coord));
    count = 0;

    while(str[*cursor] > 47 || str[*cursor] < 58) {
        coord[count] = str[*cursor];
        count++;
        *cursor++;
    }
    point.y = atof(coord);

    return point;
}



/* Example
    double t = 0;
    Usage of cubic bezier functions, if int i = 0 then start point will be included, if i = 1 first point omitted, if i < RESOLUTION_OF_t then end point omitted, if <= then endpoint included 
    for(int i = 0; i <= RESOLUTION_OF_T; i++) {
        t = ((double)1 / RESOLUTION_OF_T) * i;
        printf(" %lf %lf\n", cubic_bezier_x((double)0, (double)0, (double)1, (double)1, t),cubic_bezier_y((double)0, (double)1, (double)1, (double)0, t));
    }
*/
void decode_bezier(FILE* file, unsigned int RESOLUTION_OF_T, double start_x, double c1_x, double c2_x, double end_x, double start_y, double c1_y, double c2_y, double end_y) {
    double t = 0;
    /* Usage of cubic bezier functions, if int i = 0 then start point will be included, if i = 1 first point omitted, if i < RESOLUTION_OF_t then end point omitted, if <= then endpoint included */
    for(int i = 1; i <= RESOLUTION_OF_T; i++) {
        t = ((double)1 / RESOLUTION_OF_T) * i;
        double x = round(cubic_bezier_x(start_x, c1_x, c2_x, end_x, t));
        double y = round(cubic_bezier_y(start_y, c1_y, c2_y, end_y, t));
        move_to_point(file, x, y);
    }
}

// Num is a variable defined in main and therefore might be too small for some applications
int find_num(char* str, char* num, unsigned int* cursor, unsigned int length) {
    unsigned int i = 0;
    unsigned int index  = 0;
    if((str[*cursor] > 64) && (str[*cursor] < 91) || (str[*cursor] > 96) && (str[*cursor] < 123))
        return 1;
    do { //Migh redo this to have the functionality of finding the next num, need a filter to stop when it finds a char though or something vice versa for the find char
        if(((str[*cursor] > 47 && str[*cursor] < 58) || str[*cursor] == 46) && ((*cursor < length) && (i < 10))) {
            num[index] = str[*cursor];
            index++;
        }
        printf("inside find num %c\n", num[i]);
        i++;
        cursor++;
    } while(((str[*cursor] > 47 && str[*cursor] < 58) || str[*cursor] == 46) && ((*cursor < length) && (i < 10)));
    if(index == 0) { //If the inner if loop has executed even once(there was at least one number or dot) then this function has suceeded and index will be at least 1
        return 1;
    } else {
        *cursor = *cursor + index;
        return 0;
    }
}

int find_char(char*str, char* str_to_fill, unsigned int cursor, unsigned int length) {
    unsigned int i = 0;
    unsigned int index  = 0;
    if((str[cursor] > 47 && str[cursor] < 58) || str[cursor] == 46)
        return 1;
    do {
        if(((str[cursor] > 64) && (str[cursor] < 91) || (str[cursor] > 96) && (str[cursor] < 123)) && ((cursor < length) && (i < 10))) {
            str_to_fill[index] = str[cursor];
            index++;
        }
        i++;
        cursor++;
    
    } while(((str[cursor] > 64) && (str[cursor] < 91) || (str[cursor] > 96) && (str[cursor] < 123)) && ((cursor < length) && (i < 10)));
    if(index == 0) { //If the inner if loop has executed even once(there was at least one letter) then this function has suceeded and index will be at least 1
        return 1;
    } else {
        return 0;
    }
}
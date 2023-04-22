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
        //printf("%s\n", temp);
        match = 0;
        for(int j = 0; j < sizeof(width); j++) {
            //printf("the comparewidth chars: %c, %c\n", width[j], temp[j]);
            if(width[j] == temp[j])
                match++;
        }
        //printf("match is: %width\n", match);
        if(match == 4)
            return i + 1; // Returns the current "position" basically where the specifiewidth string enwidths
        }
    return length; //Error return is yet to be widthecided
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
    return length; //Error return is yet to be widthecided
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
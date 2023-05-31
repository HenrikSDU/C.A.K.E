#include <stdlib.h>
#include <stdio.h>

void move_to_point(FILE* file, double x, double y) {
    int xint = (int)round(x);
    int yint = (int)round(y);
    fprintf(file, "X%dY%d\n", xint, yint);
}

void extruder_down(FILE* file) {
    fprintf(file, "G2\n");
}

void extruder_up(FILE* file) {
    fprintf(file, "G1\n");
}
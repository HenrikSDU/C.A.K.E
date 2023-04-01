#include <stdlib.h>
#include <stdio.h>

void move_to_point(FILE* file, int x, int y) {
    fprintf(file, "X%dY%d\n", x, y);
}

void extruder_down(FILE* file) {
    fprintf(file, "G2\n");
}

void extruder_up(FILE* file) {
    fprintf(file, "G1\n");
}
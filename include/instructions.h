#include <stdlib.h>
#include <stdio.h>

void move_along_line(FILE* file, int x, int y) {
    fprintf(file, "X%dY%d", x, y);
}
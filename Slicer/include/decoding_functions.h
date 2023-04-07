#include <math.h>
#include <stdlib.h>

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

void str_shift_left(char* str, int length, char next) {
    char* temp = malloc((length - 1) * sizeof(char));
    for(int i = 1; i < length - 1 + 1; i++) {
        temp[i] = str[i - 1]; // temp = _asd
    }
    temp[0] = next;
    for(int i = 0; i < length - 1; i++) {
        str[i] = temp[i];
    }

    free(temp);
}
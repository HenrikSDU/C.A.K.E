#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Invalid usage. Try: *executable* *input_file* *output_file*\n");
    }
    char *arg1 = "-centerline";
    char *arg2 = malloc(sizeof(char) * sizeof(argv[1]));
    arg2 = argv[1];
    char *arg3 = "-o";
    char *arg4 = malloc(sizeof(arg2));//malloc(sizeof(char) * sizeof(argv[1]));
    arg4 = argv[2];
    char *args[] = {"autotrace.exe", arg1, arg2, arg3, arg4, NULL};

    _spawnv(_P_WAIT, "autotrace.exe", (const char * const*)args);

    /* Trying exponential function */
    double x = 0.25;
    x = pow((double)x, (double)2);
    printf("%lf\n", x);

    return 0;
}
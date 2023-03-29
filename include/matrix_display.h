#include <stdio.h>
#define DIMENSION 10

void matrix_display(int dim, unsigned char (*matrix)[DIMENSION]) {
    for(int i = 0; i < DIMENSION; i++) {
        for(int j = 0; j < DIMENSION; j++) {
            if(matrix[i][j] == 0)
                printf("O ");
            else if(matrix[i][j] == 255)
                printf("w ");
        }
        printf("\n");
    }
}
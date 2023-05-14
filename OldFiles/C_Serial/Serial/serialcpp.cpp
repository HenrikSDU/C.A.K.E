#include <stdio.h>
#include <windows.h>
#include "include/UART.h"

int main(void) {

    FILE* fp = fopen("test.txt", "r");
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fclose(fp);

    unsigned char* data = (unsigned char*)malloc(length * sizeof(unsigned char));
    int count = 0;
    while((count * 256) < length) {
        count++;
    }
    UART_Open(3, 9600, 0, 0);
    unsigned char len = length / 10.0;
    UART_SetData(&len, 1);

    for(int i = 0; i < length; i++) {
        unsigned char temp_data = data[i];
        UART_SetData(&temp_data, 1);
    }

    UART_Close();
    return 0;
}

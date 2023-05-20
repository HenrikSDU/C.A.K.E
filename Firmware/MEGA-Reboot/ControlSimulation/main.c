#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void PWM_control(unsigned char base_PWM, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {
    int x_mod;
    int y_mod;

    //Sim variables
    unsigned char x_dir[1];
    unsigned char y_dir[1];
    unsigned char x_speed = 0;
    unsigned char y_speed = 0;
    float dx = (float)x2 - (float)x1; // Unsigned chars are not good for division
    float dy = (float)y2 - (float)y1;

    float slope = 0.0;

    if(dx > 0) {
        x_dir[0] = 'F'; // Forward
        x_mod = 1; // Setting x_mod to 1 to multiply the slope by
    }
    else if(dx < 0) {
        x_dir[0] = 'B'; // Backward
        x_mod = -1; // Setting x_mod to -1 to multiply the slope by
    }
    else if(dx == 0) {
        x_dir[0] = 'S'; // Stop
        x_mod = 0; // Setting x_mod to 0 to multiply the slope by
    }

    if(dy > 0) {
        y_dir[0] = 'F';
        y_mod = 1;
    }
    else if(dy < 0) {
        y_dir[0] = 'B';
        y_mod = -1;
    }
    else if(dy == 0) {
        y_dir[0] = 'S';
        y_mod = 0;
    }

    
    if(x_dir[0] == 'S') {
        x_speed = 0;
    }
    if(y_dir[0] == 'S') {
        y_speed = 0;
    }
    if(x_mod != 0) {
        slope = x_mod * y_mod * (dy / dx); // Had problems with unsigned chars being divided so I used floats
    }

    // First try at logic controlling overflows and underflows
    if(slope > 1 && x_mod != 0 && y_mod != 0) {
        if((slope * (float)base_PWM) > 255.0) { // The idea here is to find the maximum value of base_PWM that will not overflow the OCR0A register, the limit can be decreased as needed
            while((slope * (float)base_PWM) > 255.0) {
                base_PWM--;
                printf("    base_PWM: %d\n", base_PWM);
            }
        }
        x_speed = base_PWM;
        y_speed = (int)(slope * (float)base_PWM);
    }
    else if(slope < 1 && x_mod != 0 && y_mod != 0) {
        if((slope * (float)base_PWM) > 50.0) { // The idea here is to find the minimum value of base_PWM that will not be too small to to make the motors run, while roughly achieving the target speed
            while((slope * (float)base_PWM) > 50.0) {
                base_PWM++;
                printf("    base_PWM: %d\n", base_PWM);
            }
        }
        x_speed = base_PWM;
        y_speed = (int)(slope * (float)base_PWM);
    }
    else if(slope == 1.0 && x_mod != 0 && y_mod != 0) { // Setting PWM so the motors run at roughly desired speed / sqrt(2)
        base_PWM = 100; // Some predefined value to roughly get the desired speed
        printf("    base_PWM: %d\n", base_PWM);
        x_speed = base_PWM;
        y_speed = base_PWM;
    }

    // Debug prints
    printf("x2 - x1 = %f, y2 - y1 = %f\n", dx, dy);
    printf("X direction: %c, speed: %d\n", x_dir[0], x_speed);
    printf("Y direction: %c, speed: %d\n", y_dir[0], y_speed);
    printf("Slope: %f\n", slope);
}

int main(void) {

    unsigned char desired_PWM = 100;
    unsigned char x_array[] = {2, 2, 50, 100, 60, 50, 0, 1};
    unsigned char y_array[] = {2, 2, 50, 120, 140, 10, 0, 200};

    for(int i = 0; i < 7; i++) {
        printf("FOR LOOP #%d\n", i);
        PWM_control(desired_PWM, x_array[i], x_array[i + 1], y_array[i], y_array[i + 1]);
        printf("\n");
        Sleep(200);
    }

    return 0;
}
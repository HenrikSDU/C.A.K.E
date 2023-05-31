#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

float abs_value(float x) {
    if(x < 0) {
        return -x;
    }
    else {
        return x;
    }
}

void PWM_control(unsigned char base_PWM, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2) {
    //Sim variables
    unsigned char x_dir[1];
    unsigned char y_dir[1];
    unsigned char x_speed = 0;
    unsigned char y_speed = 0;
    float dx = (float)x2 - (float)x1; // Unsigned chars are not good for division
    float dy = (float)y2 - (float)y1;
    float slope = 0.0;

    if(dx > 0) {
        //PWM_T4A_direction_change(1); // Forward
        x_dir[0] = 'F'; // Forward
    } else if(dx < 0) {
        //PWM_T4A_direction_change(0); // Backward
        x_dir[0] = 'B'; // Backward
    } else if(dx == 0) {
        x_dir[0] = 'S'; // Stop
    }

    if(dy > 0) {
        //PWM_T4B_direction_change(1); // Forward
        y_dir[0] = 'F';
    } else if(dy < 0) {
        //PWM_T4B_direction_change(0); // Backward
        y_dir[0] = 'B';
    } else if(dy == 0) {
        y_dir[0] = 'S';
    }

    dx = abs_value(dx); // Taking absolute value of dx and dy bc the sign is handled out by the direction change
    dy = abs_value(dy);

    if(x_dir[0] == 'S' || y_dir[0] == 'S') {
        if(x_dir[0] == 'S') {
            //PWM_T4A_set(0);
        }
        else if(x_dir[0] != 'S') {
            //PWM_T4A_set(base_PWM);
        }
        if(y_dir[0] == 'S') {
            //PWM_T4B_set(0);
        }
        else if(y_dir[0] != 'S') {
            //PWM_T4B_set(base_PWM);
        }
    } else {
        slope = dy / dx; // Had problems with unsigned chars being divided so I used floats

        if(slope < 1) {
            while((slope * base_PWM) < 60) {
                base_PWM++;
            }
            x_speed = base_PWM;
            y_speed = (int)(slope * (float)base_PWM);

        } else if(slope > 1) {
            while((slope * base_PWM) > 255) {
                base_PWM--;
            }
            x_speed = base_PWM;
            y_speed = (int)(slope * (float)base_PWM);

        } else if(slope == 1) {
            base_PWM = 100;
            x_speed = base_PWM;
            y_speed = base_PWM;
        }
        //PWM_T4A_set(x_speed);
        //PWM_T4B_set(y_speed);
    }
    // Debug prints
    printf("\ndx: %d - %d = %d (%f), %d - %d = %d (%f)\n", x2, x1, x2 - x1, dx, y2, y1, y2 - y1, dy);
    printf("X_dir: %c, Speed: %d\n", x_dir[0], x_speed);
    printf("Y_dir: %c, Speed: %d\n", y_dir[0], y_speed);
    printf("Slope: %f\n", slope);
}

int main(void) {

    unsigned char desired_PWM = 100;
    unsigned char x_array[] = {187, 191, 191, 44, 47, 49, 52, 55, 57, 60, 62, 65, 67, 70, 81, 92, 103, 114, 125, 135, 146, 156, 167, 177 };
    unsigned char y_array[] = {36, 126, 171, 50, 52, 54, 56, 58, 60, 62, 65, 67, 69, 71, 80, 89, 98, 107, 116, 126, 135, 145, 154, 164};
    int counter = 0;

    while(x_array[counter] != 0) {
        printf("While loop #%d\n", counter);
        PWM_control(desired_PWM, x_array[counter], x_array[counter + 1], y_array[counter], y_array[counter + 1]);
        Sleep(200);
        counter++;
    }

    return 0;
}
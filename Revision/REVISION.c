// #1: Getting Started with C Programming (Basic structure and #include directives)
// #5: Comments in C Programming (Like this one!)
#include <stdio.h>
#include <math.h> // #17: C Standard Library Functions (used for math calculations)

int main() {
    int choice;
    double angle;
    float x;
    float y;
    float z;
    int arm_length = 10; // Length of the robotic arm
    printf("1. rotate the arm at 30 degrees elevation \n");
    printf("Choose an option:\n");
    scanf("%d", &choice);
    switch(choice){
        case 1:
            printf("Rotating the arm at 30 degrees elevation...\n");
            printf("Enter the angle of rotation (in degrees): ");
            scanf("%lf", &angle);
            x=10*cos(30*M_PI/180);
            y=10*sin(30*M_PI/180)*cos(angle*M_PI/180);
            z=10*sin(30*M_PI/180)*sin(angle*M_PI/180);
            printf("The new position of the arm is: (%.2f, %.2f, %.2f)\n", x, y, z);
            break;
        case 2:
            printf("Get out u idjiot...\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break; 
    }


    return 0;
}
#include <stdio.h>

int main() {
    int daimetre;
    char choice;
    printf("do u want to test a new batch (y/n): ");
    scanf(" %c", &choice); 
    while(choice == 'y' || choice == 'Y'){
        for (int i=0;i<5;i++){
            printf("Enter the diameter of the circle: ");
            scanf("%d", &daimetre);
            if (daimetre >=49.5 && daimetre <=50.5){
                printf("The diameter is within the acceptable range.\n");
                
            } 
            else if (daimetre <=0){
                printf("Sensor error program will terminate\n");
                break;
            }
            else {
                printf("The diameter is outside the acceptable range.\n");
            }
            printf("ok\n");
        }
        printf("do u want to test a new batch (y/n): ");
        scanf(" %c", &choice); 
    }
        

}
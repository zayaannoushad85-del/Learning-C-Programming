#include <stdio.h>
int ratio;
int num;
int iteration=1;
int calculatetotalration(){
    if (iteration<=num){
        int gear;
        printf("Enter the gear ratio for the gear %d: ",iteration);
        scanf("%d",&gear);
        iteration++;
        ratio=gear*calculatetotalration();
        return ratio;
    }
    else{
        return 1;
    }
}
int main(){
    printf("enter the number of gears: ");
    scanf("%d",&num);
    int hi=calculatetotalration();
    printf("%d",hi);
    return 0;
}
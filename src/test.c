#include <stdio.h>
#include <string.h>
#include "commands.h"

int main(void){
    while(1){
        char input[128];
        printf("Enter a command: ");
        fgets(input, 128, stdin);
        input[strlen(input) - 1] = '\0';
        if(handleUserInput(input) == 1){
            break;
        }
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

char** parseInput(char* input, int* numTokens){
    char** tokens = malloc(128 * sizeof(char*));
    char* token;
    int index = 0;

    token = strtok(input, " ");
    while(token != NULL){
        tokens[index] = token;
        index++;
        token = strtok(NULL, " ");
    }
    tokens[index] = NULL;
    *numTokens = index;

    return tokens;
}

int handleUserInput(char* input, char* attribute){
    int numTokens;
    char** tokens = parseInput(input, &numTokens);
    if(numTokens == 0) return -1;

    if(strcmp(tokens[0], "select") == 0 && strcmp(tokens[1], "file") == 0 && numTokens == 3){
       strcpy(attribute, tokens[2]);
       return 1;

    } else if(strcmp(tokens[0], "send") == 0 && strcmp(tokens[1], "file") == 0 && numTokens == 2){
        return 2;

    } else if(strcmp(tokens[0], "exit") == 0 && numTokens == 1){
        return 3;

    } else if(strcmp(tokens[0], "select") == 0 && strcmp(tokens[1], "file") == 0 && numTokens == 2){
        printf("no file selected!\n");
        return -1;

    } else {

        printf("Invalid command\n");
        return -1;
    }
}

int selectFile(char* filename, FILE* file, char* buffer){
    file = fopen(filename, "r");

    if(file == NULL){
        printf("%s does not exist\n", filename);
        return -1;
    }

    if(validateExtension(filename) == -1){
        printf("%s not valid!\n", filename);
        return -1;
    }


    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    fread(buffer, 1, file_size, file);
    
    printf("%s selected\n", filename);

    fclose(file);

    return 0;
}

int validateExtension(char* filename){
    char* extension = strrchr(filename, '.');

    if(extension == NULL){
        printf("Invalid file extension\n");
        return -1;
    }

    char* validExtensions[] = {".txt", ".c", ".cpp", ".py", ".tex", ".java"};
    int numExtensions = 6;

    for(int i = 0; i < numExtensions; i++){
        if(strcmp(extension, validExtensions[i]) == 0){
            return 0;
        }
    }

    return -1;
}
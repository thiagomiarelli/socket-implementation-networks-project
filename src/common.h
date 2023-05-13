#ifndef COMMON_H
#define COMMON_H

/* ==== FUNCTIONS ==== */
char** parseInput(char* input, int* numTokens);
int handleUserInput(char* input, char* attribute);
int selectFile(char* filename, FILE* file, char* buffer);
int validateExtension(char* filename);
int sendFile(char* content, char* filename, int sockfd);
int receiveMessage(char* message, int sockfd);
int sendMessage(char* message, int sockfd);

#endif
char** parseInput(char* input, int* numTokens);
int handleUserInput(char* input, char* attribute);
int selectFile(char* filename, FILE* file, char* buffer);
int validateExtension(char* filename);
int sendFile(char* content, char* filename, int sockfd);
int receiveFile(char* message, int sockfd);
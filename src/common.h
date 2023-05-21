#ifndef COMMON_H
#define COMMON_H

/* ==== USER INPUT ==== */
void get_user_input(char* buffer, size_t size);
char** parseInput(char* input, int* numTokens);
int handleUserInput(char* input, char* attribute);

/* ==== SOCKET HELPERS ==== */
int address_parser(const char* addressString, const char* portString, struct sockaddr_storage* storage);
void addrtostr(const struct sockaddr* addr, char* str, size_t strsize);
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage* storage);

/* ==== COMMUNICATION HANDLING ==== */
int sendMessage(char* message, int sockfd);
int receiveMessage(char* message, int sockfd);

/* ==== FILE HANDLING ==== */
int validateExtension(char* filename);
int selectFile(char* filename_candidate, char* filename, FILE* file, char* buffer);
int sendFile(char* content, char* filename, int sockfd);

/* ==== ERROR HANDLING ==== */
void logexit(char *msg);


#endif
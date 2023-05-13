#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "common.h"

#define FILESIZE 512


/* ==== INPUT HANDLING ==== */
void get_user_input(char* buffer, size_t size){
    char* result = fgets(buffer, size, stdin);
    if(result == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    size_t len = strlen(buffer);
    if(len > 0 && buffer[len-1] == '\n'){
        buffer[len-1] = '\0';
    }
}

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

/* ==== SOCKET HELPERS ==== */
int address_parser(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage) {

    if (addrstr == NULL || portstr == NULL) return -1;

    uint16_t port = (uint16_t)atoi(portstr);
    if (port == 0) return -1;
    port = htons(port); 

    struct in_addr inaddr4;
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage* storage){

    uint16_t port = (uint16_t)atoi(portstr);

    if(!port) return -1;
    port = htons(port);

    memset(storage, 0, sizeof(*storage));
    if(strcmp(proto, "v4") == 0){
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return 0;
    } else if (strcmp(proto, "v6") == 0){
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        addr6->sin6_addr = in6addr_any;
        return 0;
    } else {
        return -1;
    }

    return 0;
} 

/* ==== COMMUNICATION HANDLING ==== */
int sendMessage(char* message, int sockfd){
    size_t count = 0;
    printf("sending %s\n", message);
    count = send(sockfd, message, strlen(message) + 1, 0);
    if(count != strlen(message) + 1) return -1;

    return 0;
}

int receiveMessage(char* message, int sockfd){
    size_t count = 0;
    count = recv(sockfd, message, FILESIZE-1, 0);
    if(count <= 0) return -1;

    return 0;
}

/* ==== FILE HANDLING ==== */

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

int sendFile(char* content, char* filename, int sockfd) {
    char message[FILESIZE];

    strcpy(message, filename);
    strcat(message, "\n");
    strcat(message, content);
    strcat(message, "\\end");
    
    if(sendMessage(message, sockfd) == -1) return -1;
    return 0;
}


/* ==== ERROR HANDLING ==== */
void logexit(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
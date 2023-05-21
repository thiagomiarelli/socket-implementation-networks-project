#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"


/* ==== CONSTANTS ==== */
#define MAX_FILESIZE 512
#define MAX_COMMAND_SIZE 256
#define ADDR_SIZE 128

/* ==== AUX FUNCTIONS ==== */
void usage(int argc, char *argv[]);
int setup_client(int argc, char* argv[]);

int main(int argc, char *argv[]) {

    int sockfd = setup_client(argc, argv);
    int fileSelected = 0;

    while(1){
        /* === RECEIVE COMMAND === */
        char command[MAX_COMMAND_SIZE];
        get_user_input(command, MAX_COMMAND_SIZE);

        /* === CHOOSE COMMAND === */
        char attribute[MAX_COMMAND_SIZE];
        int command_type = handleUserInput(command, attribute);

        /* === AUX VARS === */
        FILE file;
        char fileContent[MAX_FILESIZE];
        char serverResponse[MAX_FILESIZE];

        switch (command_type){
            case 1:
                {
                    int selected_file_status = selectFile(attribute, &file, fileContent);
                    if(selected_file_status == -1){
                        continue;
                    } else {
                        fileSelected = 1;
                    }
                    break;
                }
            case 2:
                {   
                    if(!fileSelected){
                        printf("no file selected!\n");
                        continue;
                    }

                    char message[MAX_FILESIZE];
                    int sendStatus = sendFile(fileContent, attribute, sockfd);
                    if(sendStatus == -1) continue;

                    memset(message, 0, MAX_FILESIZE);

                    //reseting file selection
                    memset(fileContent, 0, MAX_FILESIZE);

                    int recvStatus = receiveMessage(serverResponse, sockfd);
                    if(recvStatus == -1) continue;

                    printf("%s\n", serverResponse);
                    
                    break;
                }
            case 3:
               {
                    int sendStatus = sendMessage("exit", sockfd);
                    if (sendStatus < 0) return -1;

                    memset(serverResponse, 0, MAX_FILESIZE);
                    if(receiveMessage(serverResponse, sockfd) == -1) continue;

                    if(strcmp(serverResponse, "connection closed\n") == 0) printf("%s", serverResponse);

                    if(close(sockfd) != 0) continue;

                    return EXIT_SUCCESS;
                    break;
                }
            default:
                break;
        }
    }
    
   return EXIT_FAILURE;
}

void usage(int argc, char *argv[]) {
    printf("Usage: %s <server> <port>\n", argv[0]);
    exit(1);
}

int setup_client(int argc, char* argv[]){
    if(argc < 3) usage(argc, argv);

    /* === SETTING UP ADDRESS AND SOCKET === */

    struct sockaddr_storage storage;
    if(address_parser(argv[1], argv[2], &storage)) usage(argc, argv);

    int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sockfd < 0) logexit("socket");

    struct sockaddr *address = (struct sockaddr *)(&storage);
    socklen_t address_len = address->sa_family ==  AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

    if(0 != connect(sockfd, address, address_len)) logexit("connect");
    
    char address_string[ADDR_SIZE];
    addrtostr(address, address_string, ADDR_SIZE);
    printf("[LOG] Connected to: %s\n", address_string);

    return sockfd;
}
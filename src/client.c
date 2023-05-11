#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"
#include "commands.h"

#include <sys/socket.h>
#include <sys/types.h>

#define COMMAND_SIZE 512
#define FILESIZE 512
#define ADDR_SIZE 128

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

int main(int argc, char *argv[]) {

    int sockfd = setup_client(argc, argv);
    int fileSelected = 0;

    while(1){
        /* === RECEIVE COMMAND === */
        printf("command: ");
        char command[COMMAND_SIZE];
        get_user_input(command, COMMAND_SIZE);

        /* === CHOOSE COMMAND === */
        char attribute[COMMAND_SIZE];
        int command_type = handleUserInput(command, attribute);

        /* === AUX VARS === */
        FILE file;
        char fileContent[FILESIZE];

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

                    char message[FILESIZE];
                    int sendStatus = sendFile(fileContent, attribute, sockfd);
                    if(sendStatus == -1) continue;

                    memset(message, 0, FILESIZE);
                    unsigned total = 0;
                    size_t count = 0;

                    //reseting file selection
                    memset(fileContent, 0, FILESIZE);
                    fileSelected = 0;

                    while(1) {
                        count = recv(sockfd, message + total, FILESIZE-1, 0);
                        if (count == 0) {
                            break;
                        } else if (count < 0) {
                           logexit("recv");
                        }
                        total += count;
                    }

                    printf("%s\n", message);
                    
                    break;
                }
            case 3:
                char serverResponse[FILESIZE];

                size_t count = send(sockfd, "exit", strlen("exit") + 1, 0);
                if (count < 0) return -1;

                unsigned total = 0;
                size_t count = 0;

                while(1) {
                        count = recv(sockfd, serverResponse + total, FILESIZE-1, 0);
                        if (count == 0) {
                            break;
                        } else if (count < 0) {
                           logexit("recv");
                        }
                        total += count;
                }

                if(close(sockfd) != 0) logexit("close");
                return 0;

                break;
            default:
                continue;
                break;
        }
    }

    //fecha o socket
    close(sockfd);
}
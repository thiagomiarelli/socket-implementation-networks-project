#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"
#include "common.h"

/* ==== CONSTANTS ==== */
#define MAX_FILESIZE 512
#define MAX_COMMAND_SIZE 256


void usage(int argc, char *argv[]) {
    printf("Usage: %s <v4|v6> <server port>\n", argv[0]);
    exit(1);
}

int setup_server(int argc, char* argv[]){
    if(argc < 3) usage(argc, argv);
    const char *protocol = argv[1];

    /* ====== SETTING UP ADDRESS AND SOCKET ====== */

    struct sockaddr_storage storage;
    if(server_sockaddr_init(argv[1], argv[2], &storage) != 0) usage(argc, argv);

    int sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sockfd < 0) logexit("socket");

    int enable = 1;
    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) logexit("setsockopt");

	struct sockaddr *address = (struct sockaddr *)(&storage);
    socklen_t address_len = !strcmp(protocol, "v4") ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

    if(bind(sockfd, address, address_len) != 0) logexit("bind");
    if(listen(sockfd, 10) != 0) logexit("listen");

    char address_string[MAX_FILESIZE];
    addrtostr(address, address_string, MAX_FILESIZE);
    printf("listening on %s\n", address_string);

    return sockfd;
}

int handle_client_conections(int sockfd){
    struct sockaddr_storage client;
    struct sockaddr *clientAddress = (struct sockaddr *) &client;
    socklen_t clientAddressLen = sizeof(client);

    int clientfd = accept(sockfd, clientAddress, &clientAddressLen);
    if(clientfd == -1) logexit("accept");

    printf("client connected!\n");
    return clientfd;
}

int main(int argc, char *argv[]) {

    int sockfd = setup_server(argc, argv);
    int clientfd = handle_client_conections(sockfd);

    /* ====== ACCEPTING CONNECTIONS ====== */
    while(1){

        char message[MAX_FILESIZE];
        memset(message, 0, MAX_FILESIZE);

        if(receiveMessage(message, clientfd) < 0) logexit("receiveFile");

        printf("Received: %s\n", message);

        char acknolegment[MAX_FILESIZE];

        sprintf(acknolegment, "[LOG] Acknoledgement from: %.512s\n", message);
        
        if(sendMessage(acknolegment, clientfd) < 0) logexit("sendFile");
    }
    close(clientfd);


    exit(EXIT_SUCCESS);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include <unistd.h>


#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

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

    char address_string[BUFFER_SIZE];
    addrtostr(address, address_string, BUFFER_SIZE);
    printf("listening on %s\n", address_string);

    return sockfd;
}



int main(int argc, char *argv[]) {

    int sockfd = setup_server(argc, argv);


    /* ====== ACCEPTING CONNECTIONS ====== */
    while(1){
        struct sockaddr_storage client;
        struct sockaddr *clientAddress = (struct sockaddr *) &client;
        socklen_t clientAddressLen = sizeof(client);

        int clientfd = accept(sockfd, clientAddress, &clientAddressLen); 
        if(clientfd == -1) logexit("accept");

        char clientAddressString[BUFFER_SIZE];
        addrtostr(clientAddress, clientAddressString, BUFFER_SIZE);
        printf("[LOG] New connection from: %s\n", clientAddressString);

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        size_t count = recv(clientfd, buffer, BUFFER_SIZE-1, 0);
        printf("[MSG] %s: %s | bytes %zu\n", clientAddressString, buffer, count);

        sprintf(buffer, "[LOG] Acknoledgement from: %.1000s\n", clientAddressString);
        count = send(clientfd, buffer, strlen(buffer) + 1, 0);
        if (count != strlen(buffer) + 1) logexit("send");

        close(clientfd);
    }

    exit(EXIT_SUCCESS);
}

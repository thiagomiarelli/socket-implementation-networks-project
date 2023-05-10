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


int main(int argc, char *argv[]) {
    if(argc < 3) {
        usage(argc, argv);
    }

    //prepara o endereco do servidor
    struct sockaddr_storage storage;
    if(server_sockaddr_init(argv[1], argv[2], &storage) != 0) { //o addrparse pega o endereco, porta e armazena no storage
        usage(argc, argv); //se nao conseguir, sai do programa
    }

    //cria o socket
    int sockfd;
    sockfd = socket(storage.ss_family, SOCK_STREAM, 0); //aqui determinamos o tipo de endereco, o tipo de socket (TCP) e o protocolo
    if (sockfd < 0) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

	struct sockaddr *address = (struct sockaddr *)(&storage);

    if(bind(sockfd, address, sizeof(struct sockaddr_in)) != 0) {
        logexit("bind");
    }

    if(listen(sockfd, 10) != 0) {
        logexit("listen");
    }

    char address_string[BUFFER_SIZE];
    addrtostr(address, address_string, BUFFER_SIZE);
    printf("listening on %s\n", address_string);

    while(1){
        struct sockaddr_storage client;
        struct sockaddr *clientAddress = (struct sockaddr *) &client;
        socklen_t clientAddressLen = sizeof(client);

        int clientfd = accept(sockfd, clientAddress, &clientAddressLen); //socket do cliente

        if(clientfd == -1) {
            logexit("accept");
        }

        char clientAddressString[BUFFER_SIZE];
        addrtostr(clientAddress, clientAddressString, BUFFER_SIZE);
        printf("new connection from %s\n", clientAddressString);

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        size_t count = recv(clientfd, buffer, BUFFER_SIZE-1, 0);

        printf("[MSG] %s: %s | bytes %zu\n", clientAddressString, buffer, count);

        sprintf(buffer, "remote endpoint: %.1000s\n", clientAddressString);
        count = send(clientfd, buffer, strlen(buffer) + 1, 0);
        if (count != strlen(buffer) + 1) {
            logexit("send");
        }

        close(clientfd);
    }

    exit(EXIT_SUCCESS);
}

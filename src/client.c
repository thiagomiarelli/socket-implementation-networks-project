#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"

#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define ADDR_SIZE 128

void usage(int argc, char *argv[]) {
    printf("Usage: %s <server> <port>\n", argv[0]);
    exit(1);
}

int main(int argc, char *argv[]) {

    /* === SETUP CLIENT SERVER AND SOCK === */

    if (argc < 3) usage(argc, argv);

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

    /* === SEND AND RECEIVE MESSAGES === */

    char buffer[BUFFER_SIZE];
    printf("message: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    size_t count = send(sockfd, buffer, strlen(buffer) + 1, 0);
    if (count < 0) logexit("send");

    /* === RECEIVES ACK FROM SERVER === */
    memset(buffer, 0, BUFFER_SIZE);
    unsigned total = 0;

    while(1) {
        count = recv(sockfd, buffer + total, BUFFER_SIZE-1, 0);
        if (count == 0) {
            break;
        } else if (count < 0) {
            logexit("recv");
        }
        
        total += count;
        printf("received: %s\n", buffer);
    }

    printf("received %u bytes\n", total);
    puts(buffer);

    //fecha o socket
    close(sockfd);
}
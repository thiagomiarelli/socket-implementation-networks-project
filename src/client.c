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
    //valida o número de argumentos
    if (argc < 3) {
        usage(argc, argv);
    }

    //cria o socket
    int sockfd;

    //prepara o endereco do servidor
    struct sockaddr_storage storage;
    if(address_parser(argv[1], argv[2], &storage)) { //o addrparse pega o endereco, porta e armazena no storage
        usage(argc, argv); //se nao conseguir, sai do programa
    }

    sockfd = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logexit("socket");
    }

	struct sockaddr *address = (struct sockaddr *)(&storage);

    //conecta ao servidor
    if(0 != connect(sockfd, address, sizeof(storage))) {
        logexit("connect");
    }
    

    //obtem o endereco como string para imprimir
    char address_string[ADDR_SIZE];
    addrtostr(address, address_string, ADDR_SIZE);
    printf("connected to %s\n", address_string);


    //obtem a mensagem
    char buffer[BUFFER_SIZE];
    printf("message: ");
    fgets(buffer, BUFFER_SIZE, stdin);


    // envia mensagem
    size_t count = send(sockfd, buffer, strlen(buffer) + 1, 0);
    if (count < 0) {
        logexit("send");
    }

    //recebe resposta
    memset(buffer, 0, BUFFER_SIZE); //limpa o buffer, definindo todos os bytes como 0
    unsigned total = 0; //total de bytes recebidos, usado para controlar o recebimento parcial dos dados
    while(1) { // fica esperando pra receber a mensagem
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "common.h"

/* ==== CONSTANTS ==== */
#define MAX_FILESIZE 512
#define MAX_COMMAND_SIZE 256

/* ==== AUX FUNCTIONS ==== */
void usage(int argc, char *argv[]);
int setup_server(int argc, char* argv[]);
int handle_client_conections(int sockfd);
int break_filename_and_content(char* message, char* filename, char* content);
int check_if_end(char* message, int i);
int check_if_server_has_file(char* filename);
int save_file_on_server(char* filename, char* content);
int close_server(int sockfd);


int main(int argc, char *argv[]) {

    int sockfd = setup_server(argc, argv);
    int clientfd = handle_client_conections(sockfd);

    /* ====== ACCEPTING CONNECTIONS ====== */
    while(1){

        char message[MAX_FILESIZE];
        memset(message, 0, MAX_FILESIZE);

        if(receiveMessage(message, clientfd) < 0) logexit("receiveFile");

        if(strcmp(message, "exit") == 0){
            printf("received exit command\n");
            close_server(clientfd);
            break;
        }

        char filename[MAX_FILESIZE];
        char content[MAX_FILESIZE];

        if(break_filename_and_content(message, filename, content) < 0) logexit("break_filename_and_content");

        printf("file %s received\n", filename);

        int file_exists = check_if_server_has_file(filename);
        int save_file_status = save_file_on_server(filename, content);
        if(save_file_status < 0) logexit("save_file_on_server");

        char acknolegment[MAX_FILESIZE];

        if(file_exists == 1){
            sprintf(acknolegment, "file %s overwritten\n", filename);
        } else {
            sprintf(acknolegment, "file %s received\n", filename);
        }   
        
        if(sendMessage(acknolegment, clientfd) < 0) logexit("sendFile");
    }
    close(clientfd);

    exit(EXIT_SUCCESS);
}

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
    if(listen(sockfd, 1) != 0) logexit("listen");

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

int break_filename_and_content(char* message, char* filename, char* content){
    int i = 0;
    int string_size = strlen(message);

    while(message[i] != '\n' && i < string_size){
        filename[i] = message[i];
        i++;
    }
    if(i == string_size) return -1; 

    filename[i] = '\0';
    i++;

    if(strlen(message) - i < 5) return -1; // if there is no space to store the /end
    int j = 0;

    while(!check_if_end(message, i)){
        content[j] = message[i];
        i++;
        j++;
    }

    if(i == string_size) return -1;
    content[j] = '\0';

    return 0;
}

int check_if_end(char* message, int i){
    if(message[i] == '\\' && message[i+1] == 'e' && message[i+2] == 'n' && message[i+3] == 'd') return 1;
    return 0;
}

int check_if_server_has_file(char* filename){
    FILE *fp;
    char filename_with_dir[MAX_FILESIZE];
    sprintf(filename_with_dir, "server_files/%s", filename);
    fp = fopen(filename_with_dir, "r");
    if(fp == NULL) return 0;
    fclose(fp);
    return 1;
}

int save_file_on_server(char* filename, char* content){
    FILE *fp;
    char filename_with_dir[MAX_FILESIZE];
    sprintf(filename_with_dir, "server_files/%s", filename);
    printf("Saving file on server: %s\n", filename_with_dir);

    fp = fopen(filename_with_dir, "w");
    if(fp == NULL) return -1;
    fprintf(fp, "%s", content);
    fclose(fp);
    return 0;
}

int close_server(int sockfd){
    printf("closing server\n");
    sendMessage("connection closed\n", sockfd);
    if(close(sockfd) != 0) logexit("close");
    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "common.h"

/* ==== CONSTANTS ==== */
#define MAX_FILESIZE 512
#define MAX_COMMAND_SIZE 256

/* ==== AUX FUNCTIONS ==== */
void usage(int argc, char *argv[]);
int create_conection_thread(int clientfd);
int setup_server(int argc, char* argv[]);
int handle_client_conections(int sockfd);
int break_filename_and_content(char* message, char* filename, char* content);
int check_if_end(char* message, int i);
int check_if_server_has_file(char* filename);
int save_file_on_server(char* filename, char* content);
int close_server(int sockfd);
int validate_message(char* message);
int find_breakpoint(char* message);


int main(int argc, char *argv[]) {

    int sockfd = setup_server(argc, argv);

    while(1) {
        int clientfd = handle_client_conections(sockfd);
        int failure_disconection = 0;

        /* ====== ACCEPTING CONNECTIONS ====== */
        while(1){

            char message[MAX_FILESIZE];
            memset(message, 0, MAX_FILESIZE);

            /* ==== VALIDATIONS ==== */
            if(receiveMessage(message, clientfd) < 0){
                failure_disconection = 1;
                break;
            }

            int message_validation = validate_message(message);
            if(message_validation < 0){
                failure_disconection = 1;
                break;
            }

            
            /* ==== VALID COMMAND HANDLING ==== */

            //if command exits, close server
            if(message_validation == 1 && close_server(clientfd) == 0){
                break;
            };

            //if command is a file, save file
            char filename[MAX_FILESIZE];
            char content[MAX_FILESIZE];

            if(break_filename_and_content(message, filename, content) < 0) break;

            int file_exists = check_if_server_has_file(filename);
            int save_file_status = save_file_on_server(filename, content);

            if(save_file_status < 0){
                failure_disconection = 1;
                break;
            }

            if(file_exists == 0){
                printf("file %s received\n", filename);
            } else {
                printf("file %s overwritten\n", filename);
            }

            
            
        }

        if(failure_disconection == 1){
            close(clientfd);
        }
    }
    return EXIT_SUCCESS;
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
    return sockfd;
}

int handle_client_conections(int sockfd){
    struct sockaddr_storage client;
    struct sockaddr *clientAddress = (struct sockaddr *) &client;
    socklen_t clientAddressLen = sizeof(client);

    int clientfd = accept(sockfd, clientAddress, &clientAddressLen);
    if(clientfd == -1) logexit("accept");
    return clientfd;
}

int validate_message(char* message){
     regex_t regex;
    int reti;

    // Regular expression pattern
    char *pattern = ".+\\.(txt|cpp|c|py|tex|java).+\\\\end";

    // Compile the regular expression
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -1;
    }

    // Execute regular expression
    reti = regexec(&regex, message, 0, NULL, 0);
    if (!reti) {
        regfree(&regex);
        return 0;
    }
    else if (strcmp(message, "exit") == 0) {
        regfree(&regex);
        return 1;
    }
    else {
        regfree(&regex);
        return -1;
    } 
}

int break_filename_and_content(char* message, char* filename, char* content){
    int i = 0;
    int string_size = strlen(message);
    int breakpoint = find_breakpoint(message);

    if(breakpoint == -1) return -1;
    if(breakpoint > MAX_FILESIZE) return -1;
    while(i <= breakpoint){
        filename[i] = message[i];
        i++;
    }

    filename[i] = '\0';

    if(i == string_size) return -1; 

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

int find_breakpoint(char* message){
    int i;
    int string_size = strlen(message);
    int cut_point = -1;

    for(i = 0; i + 1 < string_size; i++){
       if(message[i] =='.' && message[i+1] == 'c') cut_point = i + 1;

       if(string_size - i >= 3) {
            if(message[i] == '.' && message[i + 1] == 'p' && message[i + 2] == 'y') cut_point = i + 2;
       }

        if(string_size - i >= 4) {
            if(message[i] == '.' && message[i + 1] == 't' && message[i + 2] == 'e' && message[i + 3] == 'x') cut_point = i + 3;
            if(message[i] == '.' && message[i + 1] == 't' && message[i + 2] == 'x' && message[i + 3] == 't') cut_point = i + 3;
            if(message[i] == '.' && message[i + 1] == 'c' && message[i + 2] == 'p' && message[i + 3] == 'p') cut_point = i + 3;
        }

        if(string_size - i >= 5) {
            if(message[i] == '.' && message[i + 1] == 'j' && message[i + 2] == 'a' && message[i + 3] == 'v' && message[i + 4] == 'a') cut_point = i + 4;
        }
    }
    return cut_point;
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
    fp = fopen(filename_with_dir, "w");
    if(fp == NULL) return -1;
    fprintf(fp, "%s", content);
    fclose(fp);
    return 0;
}

int close_server(int sockfd){
    sendMessage("connection closed\n", sockfd);
    if(close(sockfd) != 0) logexit("close");
    return EXIT_SUCCESS;
}
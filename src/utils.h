#pragma once

#include <stdlib.h>
#include <arpa/inet.h>



int address_parser(const char* addressString, const char* portString, struct sockaddr_storage* storage);
void addrtostr(const struct sockaddr* addr, char* str, size_t strsize);
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage* storage);
void logexit(char *msg);
char* get_user_input(char* buffer, size_t size);
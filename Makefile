all:
	gcc -Wall -c src/utils.c
	gcc -Wall src/client.c utils.o -o client
	gcc -Wall src/server.c utils.o -o server
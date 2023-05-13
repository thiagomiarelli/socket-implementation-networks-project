all:
	gcc -Wall -c src/utils.c
	gcc -Wall -c src/common.c
	gcc -Wall src/client.c utils.o common.o -o client
	gcc -Wall src/server.c utils.o common.o -o server

clean:
	rm -f $(CLIENT_OBJS) $(CLIENT_EXEC) $(SERVER_OBJS) $(SERVER_EXEC)
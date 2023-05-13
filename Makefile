all:
	gcc -Wall -c src/common.c
	gcc -Wall src/client.c common.o -o client
	gcc -Wall src/server.c common.o -o server

clean:
	rm -f $(CLIENT_OBJS) $(CLIENT_EXEC) $(SERVER_OBJS) $(SERVER_EXEC)
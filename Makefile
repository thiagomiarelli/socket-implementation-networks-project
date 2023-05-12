all:
	gcc -Wall -c src/utils.c
	gcc -Wall -c src/commands.c
	gcc -Wall src/client.c utils.o commands.o -o client
	gcc -Wall src/server.c utils.o commands.o -o server

clean:
	rm -f $(CLIENT_OBJS) $(CLIENT_EXEC) $(SERVER_OBJS) $(SERVER_EXEC)
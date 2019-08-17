CC = gcc
OBJS = TCP_helpers.o Aux.o
all: server client
COMP_FLAG = -std=c99 -Wall -g

TCP_helpers.o: TCP_helpers.h TCP_helpers.c  
	$(CC) $(COMP_FLAG) -c TCP_helpers.c	-o TCP_helpers.o
Aux.o: _Aux.h _Aux.c  
	$(CC) $(COMP_FLAG) -c _Aux.c -o Aux.o

server.o: server_main.c $(OBJS)
		$(CC) $(COMP_FLAG) -c server_main.c  -o server.o
		
client.o: client_main.c $(OBJS)
		$(CC) $(COMP_FLAG) -c client_main.c  -o client.o

server: server.o $(OBJS)
		$(CC) $(COMP_FLAG) server.o $(OBJS) -o file_server

client: client.o $(OBJS)
		$(CC) $(COMP_FLAG) client.o $(OBJS) -o file_client

clean:
	rm -f *.o file_server file_client

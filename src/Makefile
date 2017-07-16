CC = gcc

all : server client

server : server.o io.o common.o parse.o dsignal.o
	$(CC) -o server server.o io.o common.o parse.o dsignal.o -g# -lrt -pthread

client : client.o io.o dsignal.o
	$(CC) -o client client.o io.o dsignal.o

server.o : server.c
	$(CC) -c server.c

client.o : client.c
	$(CC) -c client.c

io.o : io.c
	$(CC) -c io.c

common.o : common.c
	$(CC) -c common.c

parse.o : parse.c
	$(CC) -c parse.c

dsignal.o : dsignal.c
	$(CC) -c dsignal.c

clean :
	rm *.exe
	rm *.o

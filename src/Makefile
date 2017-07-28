CC = gcc
CFLAGS = -g -O2 # -std=c99

all : server client

server : server.o io.o common.o parse.o dsignal.o
	$(CC) $(CFLAGS) -o server server.o io.o common.o parse.o dsignal.o -lpthread #-lrt# -lrt -pthread

client : client.o io.o dsignal.o
	$(CC) $(CFLAGS) -o client client.o io.o dsignal.o

server.o : server.c
client.o : client.c
io.o : io.c io.h
common.o : common.c common.h
parse.o : parse.c parse.h
dsignal.o : dsignal.c dsignal.h

clean :
	rm -f *.exe *.o

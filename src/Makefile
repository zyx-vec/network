CC = gcc
CFLAGS = -g -O2 # -std=c99

all : server client json thread

server : server.o io.o common.o parse.o dsignal.o log.o
	$(CC) $(CFLAGS) -o server server.o io.o common.o parse.o dsignal.o log.o -lpthread #-lrt# -lrt -pthread

client : client.o io.o dsignal.o
	$(CC) $(CFLAGS) -o client client.o io.o dsignal.o

json : json.o json_test.o
	$(CC) $(CFLAGS) -o json json_test.o json.o

thread : thread_pool.o thread_pool_test.o
	$(CC) $(CFLAGS) -o thread thread_pool_test.o thread_pool.o -lpthread

server.o : server.c
client.o : client.c
io.o : io.c io.h
common.o : common.c common.h
parse.o : parse.c parse.h
dsignal.o : dsignal.c dsignal.h
log.o : log.c log.h

json.o : json.c json.h
json_test.o : json_test.c


thread_pool.o : thread_pool.c thread_pool.h
thread_pool_test.o : thread_pool_test.c

clean :
	rm -f *.exe *.o

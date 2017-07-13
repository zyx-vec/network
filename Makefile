CC = gcc

all : server client echo_server echo_client select_echo_server

server : daytime_server.o addr_converter.o
	$(CC) -o server daytime_server.o addr_converter.o

client : client.o addr_converter.o
	$(CC) -o client client.o addr_converter.o

echo_server : echo_server.o io.o dsignal.o
	$(CC) -o echo_server echo_server.o io.o dsignal.o

echo_client : echo_client.o io.o dsignal.o
	$(CC) -o echo_client echo_client.o io.o dsignal.o

select_echo_server : select_echo_server.o io.o
	$(CC) -o select_echo_server select_echo_server.o io.o

test : test.o sock_ops.o addr_converter.o
	$(CC) -o test test.o sock_ops.o addr_converter.o -g

daytime_server.o : daytime_server.c
	$(CC) -c daytime_server.c

addr_converter.o : addr_converter.c
	$(CC) -c addr_converter.c

client.o : client.c
	$(CC) -c client.c

sock_ops.o : sock_ops.c
	$(CC) -c sock_ops.c

io.o : io.c
	$(CC) -c io.c

echo_server.o : echo_server.c
	$(CC) -c echo_server.c

echo_client.o : echo_client.c
	$(CC) -c echo_client.c

select_echo_server.o : select_echo_server.c
	$(CC) -c select_echo_server.c

dsignal.o : dsignal.c
	$(CC) -c dsignal.c

clean :
	rm *.exe
	rm *.o

all : server client echo_server echo_client

server : daytime_server.o addr_converter.o
	gcc -o server daytime_server.o addr_converter.o

client : client.o addr_converter.o
	gcc -o client client.o addr_converter.o

echo_server : echo_server.o io.o dsignal.o
	gcc -o echo_server echo_server.o io.o dsignal.o

echo_client : echo_client.o io.o dsignal.o
	gcc -o echo_client echo_client.o io.o dsignal.o

test : test.o sock_ops.o addr_converter.o
	gcc -o test test.o sock_ops.o addr_converter.o -g

daytime_server.o : daytime_server.c
	gcc -c daytime_server.c

addr_converter.o : addr_converter.c
	gcc -c addr_converter.c

client.o : client.c
	gcc -c client.c

sock_ops.o : sock_ops.c
	gcc -c sock_ops.c

io.o : io.c
	gcc -c io.c

echo_server.o : echo_server.c
	gcc -c echo_server.c

dsignal.o : dsignal.c
	gcc -c dsignal.c

clean :
	rm *.exe
	rm *.o

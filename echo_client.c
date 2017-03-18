#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"
#include "io.h"
#include "dsignal.h"

void sig_pipe(int signo) {
	fprintf(stderr, "%s\n", "SIGPIPE generated.");
	exit(1);
	return;
}

void doit(FILE* fp, int fd) {

	Signal(SIGPIPE, sig_pipe);

	char sendline[MAXLINE], recvline[MAXLINE];

	// when server process terminated, client blocked here gets()
	// when the user input some to the fp, then fgets returns.
	// But the server process has terminated, so write to a socket
	// that have recived a FIN could result the server process(kernel)
	// to generate a RST to this client.
	while(fgets(sendline, MAXLINE, fp) != NULL) {
		// the first write to elicit RST when server process terminated.
		// It could elicit a RST from server.
		writen(fd, sendline, 1);
		// sleep one second.
		sleep(1);
		// Write to a socket that have recived a RST packet, result into
		// SIGNAL SIGPIPE to be generated.
		writen(fd, sendline+1, strlen(sendline)-1);

		if((readline(fd, recvline, MAXLINE)) == 0) {
			fprintf(stderr, "%s\n", "doit() server terminated prematurely");
		}
		fputs(recvline, stdout);
	}
}

#if 0

int main(int argc, char** argv) {

	if(argc != 3) {
		fprintf(stdout, "Usage: echo_client [ip] [port]");
		exit(1);
	}

	int clientfd[5], len;
	struct sockaddr_in server;

	// 5 socket connections established, so when client terminated,
	// causing server to terminate 5 child process, means 5 children
	// send SIGCHLD to parent server almost at the same time, you
	// could notice that there may don't have 5 printf on the server.
	// check it out using ps command, there still have several child
	// processes stay at zombie state. (Signal can't be queued!!!)
	// The Unix Network Programming page 152.
	// server only call to wait is not enough!!!
	for(int i = 0; i < 5; i++) {
		if((clientfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			fprintf(stderr, "%s\n", "socket() error");
			exit(1);
		}

		bzero(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(atoi(argv[2]));
		if((inet_pton(AF_INET, argv[1], &server.sin_addr)) <= 0) {
			fprintf(stderr, "%s\n", "inet_pton() error");
			exit(1);
		}

		if((connect(clientfd[i], (SA*)&server, sizeof(server))) < 0) {
			fprintf(stderr, "%s\n", "connect() error");
			exit(1);
		}
	}

	doit(stdin, clientfd[0]);

	return 0;
}

#endif

#if 1
int main(int argc, char** argv) {

	if(argc != 3) {
		fprintf(stdout, "Usage: echo_client [ip] [port]");
		exit(1);
	}

	int clientfd, len;
	struct sockaddr_in server;

	if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		// socket
		fprintf(stderr, "%s\n", "socket() error");
		exit(1);
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	if((inet_pton(AF_INET, argv[1], &server.sin_addr)) <= 0) {
		fprintf(stderr, "%s\n", "inet_pton() error");
		exit(1);
	}


	if((connect(clientfd, (SA*)&server, sizeof(server))) < 0) {
		fprintf(stderr, "%s\n", "connect() error");
		exit(1);
	}

	doit(stdin, clientfd);

	return 0;
}
#endif
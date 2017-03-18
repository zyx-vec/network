#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"
#include "io.h"
#include "dsignal.h"

void sig_chld(int signo) {
	pid_t pid;
	int stat;

	// pid = wait(&stat);
	// // wait it child to clean it up. But wait is not enough,
	// // because if several signals come almost at the same
	// // time, only have one or two signal was handled, result
	// // to other signals be ignored by the kernel.
	// // SIGNAL CAN'T BE QUEUED!
	// fprintf(stdout, "child %d terminated.\n", pid);

	while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		fprintf(stdout, "child %d terminated.\n", pid);
	}

	return;
	// may interrupt a current blocked system call, like
	// accept() of the parent process. so we need handle
	// it, when it happends. see below calls to accept().
}

int echo(int fd) {

	char buff[MAXLINE];
	size_t n;

	for(;;) {
		if((n = readline(fd, buff, MAXLINE)) < 0) {
			fprintf(stderr, "%s\n", "readline() error");
			return -1;
		}
		else if(n == 0) {	// client close it.
			return 0;
		}

		if((writen(fd, buff, n)) < 0) {
			fprintf(stderr, "%s\n", "writen() error");
			return -1;
		}
	}

	return 0;
}

int add(int fd) {
	char buff[MAXLINE];
	size_t n;

	long arg1, arg2;

	for(;;) {
		if((n = readline(fd, buff, MAXLINE)) < 0) {
			fprintf(stderr, "%s\n", "readline() error");
			return -1;
		}
		else if(n == 0) {
			return 0;
		}

		if(sscanf(buff, "%ld%ld", &arg1, &arg2) == 2) {
			snprintf(buff, sizeof(buff), "%ld\n", arg1+arg2);
		} else {
			snprintf(buff, sizeof(buff), "input error.\n");
		}

		if((writen(fd, buff, strlen(buff))) < 0) {
			fprintf(stderr, "%s\n", "writen() error");
			return -1;
		}
	}

	return 0;
}

int main() {
	int listenfd, connfd;

	struct sockaddr_in server, client;
	int len = sizeof(client);

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		// socket
		fprintf(stderr, "%s\n", "socket() error");
		exit(1);
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(ECHO_SERVER_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if((bind(listenfd, (SA*)&server, sizeof(server))) != 0) {	// bind
		fprintf(stderr, "%s\n", "bind() error");
		exit(1);
	}

	if((listen(listenfd, BACKLOG)) != 0) {						// listen
		fprintf(stderr, "%s\n", "listen() error");
		exit(1);
	}

	// when child process become zombie state, parent server process clean it up
	// child could send SIGCHLD signal to parent when it terminates.
	Signal(SIGCHLD, sig_chld);

	for(;;) {
		len = sizeof(client);
		// connfd = accept(listenfd, (SA*)&client, &len);			// accept
		if((connfd = accept(listenfd, (SA*)&client, &len)) < 0) {
			if(errno == EINTR) {
				continue;
			} else {
				fprintf(stderr, "%s\n", "accept() error");
				exit(1);
			}
		}

		if((fork()) == 0) {
			close(listenfd);

			// if((echo(connfd)) != 0) {
			if((add(connfd)) != 0) {
				fprintf(stderr, "%s\n", "echo() error");
				exit(1);
			}

			fprintf(stderr, "%s\n", "client closed");
			close(connfd);
			exit(0);
		}
		close(connfd);
	}

	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <semaphore.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <time.h>

#include "common.h"
#include "io.h"
#include "dsignal.h"


#define SEM_PATH "./log/sem_http_log"

#define LOG_PATH "./log/log.txt" 

char* content = "HTTP/1.0 200 OK\r\n\
Content-type: text/plain\r\n\
Content-length: 19\r\n\r\n\
Hi! I\'m a message!";


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

int write_log(struct sockaddr_in* addr) {
    time_t cur_time;
    char log_buff[64];
    char* ptr = log_buff;
    struct tm* cur_tm;
    memset(log_buff, 0, sizeof(log_buff));
    char* ip = inet_ntoa(addr->sin_addr);
    
    sem_t* sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    sem_wait(sem_id);

    // must specify the third parameter: mode, because O_CREAT is here.
    int fd = open(LOG_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "open log file failed, in file %s, line %d\n", __FILE__, __LINE__);
        return -1;
    }
    cur_time = time(NULL);
    cur_tm = localtime(&cur_time);
    strftime(log_buff, sizeof(log_buff)-1, "%F %T, client's ip: ", cur_tm);
    ptr = strcat(ptr, ip);
    printf("log: %s\n", ptr);

    write(fd, ptr, strlen(ptr));
    write(fd, "\r\n", 2);

    sem_post(sem_id);
    sem_close(sem_id);
    sem_unlink(SEM_PATH);
    close(fd);
    return 0;
}

int http_serve(int fd, struct sockaddr_in* addr) {

	char buff[MAXHEAD];
	size_t n;

    n = get_http_request(fd, buff, MAXLINE);
    printf("%s", buff);
    if (n < 2 || (write_log(addr) < 0)) {
        return -1;
    }
    int response_length = strlen(content);
    if((writen(fd, content, response_length)) < 0) {
        fprintf(stderr, "writen() error, file: %s, line: %d\n", __FILE__, __LINE__);
        return -1;
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
				fprintf(stderr, "accept() error, file: %s, line: %d\n", __FILE__, __LINE__);
				exit(1);
			}
		}

		if((fork()) == 0) {
			close(listenfd);

			if((http_serve(connfd, &client)) != 0) {
			//if((add(connfd)) != 0) {
				fprintf(stderr, "echo() error, file: %s, line: %d\n", __FILE__, __LINE__);
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "common.h"
#include "io.h"


int main() {

    int listenfd, connfd, sockfd, client[FD_SETSIZE];
    int i, maxi, maxfd, nready, n;

    fd_set rset, allset;
    socklen_t clilen;

    char buff[MAXLINE];

    struct sockaddr_in server, client_addr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s\n", "socket() error");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if((bind(listenfd, (SA*)&server, sizeof(server))) < 0) {
        fprintf(stderr, "%s\n", "bind() error");
        exit(1);
    }

    if((listen(listenfd, BACKLOG)) < 0) {
        fprintf(stderr, "%s\n", "listen() error");
        exit(1);
    }

    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for(;;) {
        rset = allset;
        if((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            fprintf(stderr, "%s\n", "select() error");
            exit(1);
        }

        if(FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(client_addr);
			printf("$$$$$$$$$$$$$$$$$\n");
            if((connfd = accept(listenfd, (SA*)&client_addr, &clilen)) < 0) {
                fprintf(stderr, "%s\n", "accept() error");
                exit(1);
            }

            for(i = 0; i < FD_SETSIZE; i++) {
                if(client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if(i == FD_SETSIZE) {
                fprintf(stderr, "%s\n", "too many clients");
                exit(1);
            }

            FD_SET(connfd, &allset);
            if(connfd > maxfd) {
                maxfd = connfd;
            }
            if(i > maxi) {
                maxi = i;
            }

            if(--nready <= 0) {
                continue;
            }
        }

        for(i = 0; i <= maxi; i++) {
            if((sockfd = client[i]) < 0) {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) {
                if((n = Read(sockfd, buff, MAXLINE)) == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    writen(sockfd, buff, n);
                }

                if(--nready <= 0) {
                    break;
                }
            }
        }
    }

    return 0;
}

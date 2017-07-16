#include <stdio.h>
#include <stdlib.h>		// exit
#include <string.h>		// bzero, bcmp, bcpy
#include <sys/types.h>	// types
#include <sys/socket.h>	// socket
#include <arpa/inet.h>	// sizeof(struct sockaddr_in)
#include <netinet/in.h>	// htonl ...
#include <unistd.h>		// write
#include <time.h>


#define BUFFSIZE 1024

#define BACK_LOG 16

void client_info(struct sockaddr_in* client, int size) {
    fprintf(stdout, "%s\t%d\n", inet_ntoa(client->sin_addr), size);
    // new version address converting functions
    char buff[INET_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET, &(client->sin_addr), buff, strlen(buff));
    if(p) {
        fprintf(stdout, "%s\n", p);
    }
}

int main() {
    int port = 13, listenfd, connfd;
    struct sockaddr_in sock;
    time_t ticks;
    char buff[BUFFSIZE];

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "%s\n", "socket() error");
        exit(1);
    }

    bzero(&sock, sizeof(struct sockaddr_in));

    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    if((bind(listenfd, (struct sockaddr*)&sock, sizeof(struct sockaddr_in))) < 0) {
        fprintf(stderr, "%s\n", "bind() error");
        exit(1);
    }

    if((listen(listenfd, BACK_LOG)) < 0) {
        fprintf(stderr, "%s\n", "listen() error");
        exit(1);
    }

    struct sockaddr_in client;
    int size;
    for(;;) {
        if((connfd = accept(listenfd, (struct sockaddr*)&client, &size)) < 0) {
            fprintf(stderr, "%s\n", "accept() error");
            exit(1);
        }
        client_info(&client, size);

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        // move app data segment to kernel, then kernel is responsiable for sending this data.
        // write returning does't mean reciver recived this data segment.
        if((write(connfd, buff, strlen(buff))) < 0) {
            fprintf(stderr, "%s\n", "write() error");
            exit(1);
        }

        close(connfd);
    }


    return 0;
}

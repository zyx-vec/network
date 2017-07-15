#include <stdio.h>
#include <sys/types.h>	// uint8_t ...
#include <arpa/inet.h>	// inet_aton, inet_ntoa, sizeof(struct sockaddr_in)
#include <sys/socket.h>
#include <netinet/in.h>	// htons, htonl, ntohs, ntohl
#include <string.h>		// bzero, bcmp, bcpy
#include <unistd.h>

#include "addr_converter.h"

/*
 * struct in_addr {
 *   in_addr_t s_addr;	
 * }
 */


int main() {

    struct in_addr a;
    char* ip = "127.0.0.1";

    // inet_aton
    if(!(inet_aton(ip, &a))) {
        fprintf(stderr, "%s\n", "inet_aton error");
    } else {
        fprintf(stdout, "%s\n", "inet_aton succeed");
        uint8_t* p = (uint8_t*)(&a);
        p = (uint8_t*)(&(a.s_addr));
        for(size_t i = 0; i < sizeof(struct in_addr); i++) {
            printf("%.1x ", *(p++));	// print each byte of in_addr a, big endian.
        }
    }
    printf("\n");

    // inet_addr, deprecated
    in_addr_t a1 = inet_addr(ip);
    if(a1 == INADDR_NONE) {
        fprintf(stderr, "%s\n", "inet_addr error");
    } else {
        fprintf(stdout, "%s\n", "inet_addr succeed");
        uint8_t* p = (uint8_t*)(&a1);
        for(size_t i = 0; i < sizeof(in_addr_t); i++) {
            printf("%.1x ", *(p++));
        }
    }
    printf("\n");

    // inet_ntoa, return a static string, can't reenter to it.
    char* s = inet_ntoa(a);
    if(s != NULL) {
        fprintf(stdout, "ip: %s\n", s);
    }

    uint32_t ip_t = *(uint32_t*)(&a);    // big endian
    fprintf(stdout, "%.8x\n", ip_t);

    uint32_t ip_little = ntohl(ip_t);
    fprintf(stdout, "%.8x\n", ip_little);

    uint32_t ip_big = htonl(ip_little);
    fprintf(stdout, "%.8x\n", ip_big);

    char* s0 = "Hello";
    char* s1 = "Hello";
    if(!(bcmp(s0, s1, strlen(s0)))) {
        fprintf(stdout, "%s\n", "equal");
    }

    // server socket
    struct sockaddr_in sock;
    bzero(&sock, sizeof(struct sockaddr_in));
    uint16_t port = 13;

    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    if(!(inet_aton(ip, &(sock.sin_addr)))) {
        fprintf(stderr, "%s\n", "inet_aton error");
    }
    // self written Inet_ntop
    char* tmp;
    if((tmp = Inet_ntop((struct sockaddr*)&sock, sizeof(sock))) != NULL) {
        fprintf(stdout, "%s\n", tmp);
    }

    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)    // get a client socket descriptor in kernel space, sockfd refer to it.
        fprintf(stderr, "%s\n", "socket() error");

    if(connect(sockfd, (struct sockaddr*)&sock, sizeof(sock)) < 0)    // generic socket.
        fprintf(stderr, "%s\n", "connect() error");

    int n;
    char buff[1024];
    while((n = read(sockfd, buff, 1024)) > 0) {    // server close it, read will return 0, means four way handshake weave.
        buff[n] = 0;
        if(fputs(buff, stdout) == EOF)
            fprintf(stderr, "%s\n", "fputs error");
    }

    if(n < 0)    // if read returns negative value, means error happened
        fprintf(stderr, "%s\n", "read error");

    return 0;
}

// CONTROL + ENTER, insert a newline at below of current line.
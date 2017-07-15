#include <string.h>		// bzero, bcmp, bcpy
#include <sys/types.h>	// types
#include <sys/socket.h>	// socket
#include <arpa/inet.h>	// sizeof(struct sockaddr_in)
#include <netinet/in.h>	// htonl ...
#include <errno.h>

#include "sock_ops.h"

// returns: 0 if OK, -1 on error
int sock_bind_wild(int sockfd, int family) {
    // http://www.masterraghu.com/subjects/np/introduction/unix_network_programming_v1.3/ch04lev1sec4.html
    switch(family) {
        case AF_INET: {
            struct sockaddr_in sa4;	// TODO: using sock_bind_wild function
            bzero(&sa4, sizeof(struct sockaddr_in));
            sa4.sin_family = AF_INET;
            sa4.sin_port = htons(0);
            sa4.sin_addr.s_addr = htonl(INADDR_ANY);
            if((bind(sockfd, (struct sockaddr*)&sa4, sizeof(struct sockaddr_in))) < 0) {
                return -1;
            }
            return 0;
        }

        case AF_INET6: {
            struct sockaddr_in6 sa6;
            bzero(&sa6, sizeof(struct sockaddr_in6));
            sa6.sin6_family = AF_INET6;
            sa6.sin6_port = htons(0);
            sa6.sin6_addr = in6addr_any;
            if((bind(sockfd, (struct sockaddr*)&sa6, sizeof(struct sockaddr_in6))) < 0) {
                return -1;
            }
            return 0;
        }

        default: errno = EAFNOSUPPORT;
    }
    return -1;
}

// returns: 0 if addresses are of the same family and equal, else nonzero
int sock_cmp_addr(const struct sockaddr* sa1,
                  const struct sockaddr* sa2, socklen_t addrlen) {

    if(sa1->sa_family == sa2->sa_family) {
        if(sa1->sa_family == AF_INET)
            return bcmp(&SOCKADDR_IN(sa1)->sin_addr.s_addr,
                        &SOCKADDR_IN(sa2)->sin_addr.s_addr,
                        sizeof(SOCKADDR_IN(sa1)->sin_addr.s_addr));
        else
            return bcmp(&SOCKADDR_IN6(sa1)->sin6_addr.s6_addr,
                        &SOCKADDR_IN6(sa2)->sin6_addr.s6_addr,
                        sizeof(SOCKADDR_IN6(sa1)->sin6_addr.s6_addr));
    }
    return -1;
}

// returns: 0 if addresses are of the same family and ports are equal, else nonzero
int sock_cmp_port(const struct sockaddr* sa1,
                  const struct sockaddr* sa2, socklen_t addrlen) {
    // if(sa1.sin_family == sa2.sin_family) {
    //     return bcmp(&sa1.sin_port, &sa2.sin_port, sizeof(sa1.sin_port));
    // }
    return -1;
}

// returns: non-null pointer if OK, NULL on error
char* sock_ntop_host(const struct sockaddr* sa, socklen_t addrlen) {
    return NULL;
}

void sock_set_addr(const struct sockaddr* sa, socklen_t addrlen, void* ptr) {
    // uint8_t* p = (uint8_t*)&sa->sin_addr.s_addr;
    // for(int i = 0; i < addrlen; i++) {
    //     *(uint8_t*)ptr++ = *p++;
    // }
}

void sock_set_port(const struct sockaddr* sa, socklen_t addrlen, int* port) {
    // *port = sa->sin_port;
}

int sock_set_wild(struct sockaddr* sa, socklen_t addrlen) {
    // switch(sa->sin_family) {
    //     case AF_INET: {
    //         bzero(sa, sizeof(struct sockaddr_in));
    //         sa->sin_family = AF_INET;
    //         sa->sin_port = htons(0);
    //         sa->sin_addr.s_addr = htonl(INADDR_ANY);
    //         return 0;
    //     }
    //     case AF_INET6: {
    //         bzero(sa, sizeof(struct sockaddr_in6));
    //         sa->sin_family = AF_INET6;
    //         sa->sin_port = htons(0);
    //         sa->sin_addr.s_addr = in6addr_any;
    //         return 0;
    //     }
    //     default: {
    //         errno = EAFNOSUPPORT;
    //         return -1;
    //     }
    // }
}
#ifndef SOCK_OPS_H_

#define SOCK_OPS_H_

#include <sys/types.h>

#define SOCKADDR_IN(sa)		((struct sockaddr_in*)sa)
#define SOCKADDR_IN6(sa)	((struct sockaddr_in6*)sa)

// returns: 0 if OK, -1 on error
int sock_bind_wild(int sockfd, int family);

// returns: 0 if addresses are of the same family and equal, else nonzero
int sock_cmp_addr(const struct sockaddr* sa1,
                  const struct sockaddr* sa2, socklen_t addrlen);

// returns: 0 if addresses are of the same family and ports are equal, else nonzero
int sock_cmp_port(const struct sockaddr* sa1,
                  const struct sockaddr* sa2, socklen_t addrlen);

// returns: non-null pointer if OK, NULL on error
char* sock_ntop_host(const struct sockaddr* sa, socklen_t addrlen);

void sock_set_addr(const struct sockaddr* sa, socklen_t addrlen, void* ptr);

void sock_set_port(const struct sockaddr* sa, socklen_t addrlen, int* port);

int sock_set_wild(struct sockaddr* sa, socklen_t addrlen);

#endif
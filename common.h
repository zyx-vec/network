#ifndef COMMON_H_

#define COMMON_H_

#define SA struct sockaddr

#define BACKLOG 13
#define ECHO_SERVER_PORT 5678

#define MAXLINE 256

// signal.c
typedef void sigfunc(int);

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

#endif
#ifndef COMMON_H_

#define COMMON_H_

#define SA struct sockaddr

#define KiB 1024
#define MiB (1024*KiB)

#define BACKLOG 13
#define ECHO_SERVER_PORT 5678

#define MAXLINE 256
#define MAXHEAD (8*KiB)
#define MAXENTITYBODY (2*MiB)

#define NEWLINE "\r\n"

// ERROR code
#define E_URI_OUTRANGE -414

// signal.c
typedef void sigfunc(int);

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

#define DEBUG(msg) \
    fprintf(stderr, msg" error, file: %s, line: %d\n", __FILE__, __LINE__)


#define URI_OUTRANGE_RESPONSE \
    "HTTP/1.0 414 Request-URI Too Long\r\nContent-type: text/plain\r\nContent-length: 19\r\n\r\nError! URI to long"

int http_str2int(char* p);

#endif

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

#define HTML ".html"

// ERROR code
#define E_NOT_FOUND     -404
#define E_URI_OUTRANGE  -414

// signal.c
typedef void sigfunc(int);

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

#define DEBUG(msg) \
    fprintf(stderr, msg" error, file: %s, line: %d\n", __FILE__, __LINE__)


#define URI_OUTRANGE_RESPONSE \
    "HTTP/1.0 414 Request-URI Too Long\r\nContent-type: text/plain\r\nContent-length: 19\r\n\r\nError! URI to long"

int http_str2int(char* p);
int short2str(short n, char* p);

struct head_t;
struct arg_t;

struct request_t {
    char* url[3];
    struct head_t* head;
    int size;
    union {
        struct arg_t* args;
        char* entity_body;
    } u;
};

struct head_t {
    int num_of_head_lines;
    char** lines;
};

struct arg_t {
    char* key;
    char* value;
};

#endif

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
#include <assert.h>

#include "common.h"
#include "io.h"
#include "dsignal.h"
#include "parse.h"


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
    char log_buff[512];
    char* ptr = log_buff;
    struct tm* cur_tm;
    memset(log_buff, 0, sizeof(log_buff));
    char* ip = inet_ntoa(addr->sin_addr);
    short port = ntohs(addr->sin_port);
    
    // sem_t* sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    // sem_wait(sem_id);

    // must specify the third parameter: mode, because O_CREAT is here.
    int fd = open(LOG_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        DEBUG("open");
        return -1;
    }
    cur_time = time(NULL);
    cur_tm = localtime(&cur_time);
    strftime(log_buff, sizeof(log_buff)-1, "%F %T, client's ip: ", cur_tm);
    ptr = strcat(ptr, ip);
    char port_s[9];
    snprintf(port_s, sizeof(port_s), ":%d\r\n", port);
    ptr = strcat(ptr, port_s);
    printf("log: %s\n", ptr);

    write(fd, ptr, strlen(ptr));    // One system is atomic, use two seperate write here could cause race condition
    // write(fd, "\r\n", 2);

    // sem_post(sem_id);
    // sem_close(sem_id);
    // sem_unlink(SEM_PATH);
    close(fd);
    return 0;
}

int send2(int fd, const char* response, int length) {
    if((writen(fd, response, length)) < 0) {
        DEBUG("writen");
        return -1;
    }
    return 0;
}

int send_response(int fd, struct request_t* request) {
    assert(request != NULL);
    printf("send response: %s\n", request->url[1]);
}

int http_post(int fd, struct request_t* request) {
    int ret = 0;
    char* entity_body;
    int content_length, n;
    char* response;
    int response_length;

    content_length = parse_http_content_length(request);
    // printf("content-length: %d\n", content_length);
    entity_body = (char*)malloc(content_length+1);
    n = get_http_entity_body(fd, entity_body, content_length);
    printf("%s\n", entity_body);
    request->u.entity_body = entity_body;
    request->size = content_length;

    response = content;
    response_length = strlen(content);
    if(send2(fd, response, response_length) < 0) {
        ret = -1;
    }

    return ret;
}

int http_get(int fd, struct request_t* request) {
    int ret = 0, response_length, suffix_length=0;
    char* type;
    char* response = content;
    response_length = strlen(content);
    
    type = parse_http_url_type(request->url[1]);
    suffix_length = strlen(type);
    
    char* filename = (char*)malloc(strlen(request->url[1]));
    memset(filename, 0, strlen(request->url[1]));
    if(parse_http_request_filename(filename, request) < 0) {
        DEBUG("parse_http_request_filename");
        return -1;
    }
    int file;
    struct stat filestat;
    char filesize[7];   // attacker!
    if((file = open(filename, O_RDONLY)) >= 0 && ((fstat(file, &filestat)) >= 0)) {
        response_length = filestat.st_size;
        printf("file size: %d\n", response_length);
        sprintf(filesize, "%zd\r\n", response_length);
        response = (char*)malloc(MAXHEAD);
        response[0] = '\0';
        strcat(response, "HTTP/1.1 200 OK\r\nContent-Length: ");
        strcat(response, filesize);
        strcat(response, "Content-Type: ");
        strcat(response, type);
        strcat(response, "\r\n");
        strcat(response, "Connection: keep-alive\r\n\r\n");
        if (writen(fd, response, strlen(response)) < 0) {
            DEBUG("writen");
            free(response);
            return -1;
        }

        char* buff = (char*)malloc(response_length);
        if((readn(file, buff, response_length)) != response_length) {
            DEBUG("readn");
            ret = -1;
        } else {
            if (writen(fd, buff, response_length) != response_length) {
                DEBUG("writen");
                ret = -1;
            }
        }

        free(response);
        free(buff);
    } else {
        DEBUG("open & fstat");
        ret = -1;
    }

    return ret;
}

int http_serve(int fd, struct sockaddr_in* addr) {
    static int C = 0;

    int ret = 0;
    char buff[MAXHEAD]; // stackoverflow!!!
    size_t n, response_length;

    const char* ptr;  // make ptr point to response content
    int num_of_line = 0;

    n = get_http_request(fd, buff, MAXLINE, &num_of_line);
    if (n < 2 || (write_log(addr) < 0)) {
        if (n == E_URI_OUTRANGE) {
            response_length = strlen(URI_OUTRANGE_RESPONSE);
            ptr = URI_OUTRANGE_RESPONSE;
            send2(fd, ptr, response_length);
        }
        else {
            return -1;
        }
    }

    struct request_t request;
    struct head_t head;
    memset(&request, 0, sizeof(struct request_t));
    memset(&head, 0, sizeof(struct head_t));

    head.num_of_head_lines = num_of_line - 2;
    head.lines = (char**)malloc(head.num_of_head_lines * sizeof(char*));
    request.head = &head;
    if (parse_http_request(buff, &request) != 0) {
        DEBUG("parse_http_request");
        ret = -1;
    } else {
        if (!strcmp(request.url[0], "POST")) {
            if (http_post(fd, &request) < 0) {
                DEBUG("http_post");
                ret = -1;
            }
        } else if (!strcmp(request.url[0], "GET")) {
            if (http_get(fd, &request) < 0) {
                DEBUG("http_get");
                ret = -1;
            }
        }
    }


    if (parse_http_is_keep_alive(&request)) {
        printf("********Keep-alive********\n");
        free(head.lines);
        if (!strcmp(request.url[0], "GET"))
            free(request.u.args);
        else if (!strcmp(request.url[0], "POST"))
            free(request.u.entity_body);

        //TODO: set up a timer
        return http_serve(fd, addr);
    } else {
        free(head.lines);
        if (!strcmp(request.url[0], "GET"))
            free(request.u.args);
        else if (!strcmp(request.url[0], "POST"))
            free(request.u.entity_body);

        return ret;
    }
}

int add(int fd) {
    char buff[MAXLINE];
    size_t n;

    long arg1, arg2;

    for(;;) {
        if((n = readline(fd, buff, MAXLINE)) < 0) {
            DEBUG("readline");
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
            DEBUG("writen");
            return -1;
        }
    }

    return 0;
}

int main() {
    int listenfd, connfd, count = 0;

    struct sockaddr_in server, client;
    int len = sizeof(client);

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {        // socket
        DEBUG("socket");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(ECHO_SERVER_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if((bind(listenfd, (SA*)&server, sizeof(server))) != 0) {    // bind
        DEBUG("bind");
        exit(1);
    }

    if((listen(listenfd, BACKLOG)) != 0) {                        // listen
        DEBUG("listen");
        exit(1);
    }

    // when child process become zombie state, parent server process clean it up
    // child could send SIGCHLD signal to parent when it terminates.
    Signal(SIGCHLD, sig_chld);
    
    for(;;) {
        len = sizeof(client);
        // connfd = accept(listenfd, (SA*)&client, &len);        	// accept
        count += 1;
        if((connfd = accept(listenfd, (SA*)&client, &len)) < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                DEBUG("accept");
                exit(1);
            }
        }

        // 1344415204
        if((fork()) == 0) {
            close(listenfd);

            if((http_serve(connfd, &client)) != 0) {
            //if((add(connfd)) != 0) {
                DEBUG("http_serve");
                exit(1);
            }

            fprintf(stderr, "%s\n", "client closed");
            close(connfd);
            exit(0);
        }
        close(connfd);
        printf("TOTAL number of request: %d\n", count);
    }


    return 0;
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "common.h"

static pthread_mutex_t mutex;
static pthread_once_t once = PTHREAD_ONCE_INIT;

void mutex_init() {
    pthread_mutex_init(&mutex, NULL);
}

int write_log(struct sockaddr_in* addr) {
    pthread_once(&once, mutex_init);

    time_t cur_time;
    char log_buff[512];
    char* ptr = log_buff;
    struct tm* cur_tm;
    memset(log_buff, 0, sizeof(log_buff));

    pthread_mutex_lock(&mutex);
    char* ip = inet_ntoa(addr->sin_addr);
    short port = ntohs(addr->sin_port);
    
    // sem_t* sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    // sem_wait(sem_id);

    // must specify the third parameter: mode, because O_CREAT is here.
    int fd = open(LOG_PATH, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    // TODO: why open failed after a baunch request handled?
    if (fd == -1) {
        DEBUG("open");
        pthread_mutex_unlock(&mutex);
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

    // One system is atomic, use two seperate write here could cause race condition
    if (write(fd, ptr, strlen(ptr)) == -1) {
        DEBUG("log error, write");
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    // write(fd, "\r\n", 2);

    // sem_post(sem_id);
    // sem_close(sem_id);
    // sem_unlink(SEM_PATH);
    close(fd);
    pthread_mutex_unlock(&mutex);
    return 0;
}


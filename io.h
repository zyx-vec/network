#ifndef IO_H_

#define IO_H_

ssize_t readn(int fd, void* buff, size_t n);

ssize_t writen(int fd, const void* buff, size_t n);

ssize_t readline(int fd, void* buff, int maxlen);

ssize_t Read(int fd, void *ptr, size_t nbytes);

void Write(int fd, void *ptr, size_t nbytes);

ssize_t get_http_request(int fd, void* ptr, int maxline);

#endif

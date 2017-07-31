#ifndef __IO_H

#define __IO_H

ssize_t readn(int fd, void* buff, size_t n);

ssize_t writen(int fd, const void* buff, size_t n);

ssize_t readline(int fd, void* buff, int maxlen);

ssize_t Read(int fd, void *ptr, size_t nbytes);

void Write(int fd, void *ptr, size_t nbytes);

ssize_t get_http_request(int fd, void* ptr, int maxline, int* num_of_line);

ssize_t get_http_entity_body(int fd, void* ptr, int nbytes);

#endif

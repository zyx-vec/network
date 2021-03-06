#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "common.h"

ssize_t readn(int fd, void* buff, size_t n) {
    char* p = (char*)buff;

    size_t nleft, nread;
    nleft = n;

    while(nleft > 0) {
        if((nread = read(fd, p, nleft)) < 0) {
            if(errno == EINTR) {
                nread = 0;
            } else {
                return -1;
            }
        } else if(nread == 0) {	// the other end closed socket
            break;
        }
        nleft -= nread;
        p += nread;
    }

    return n-nleft;
}

ssize_t writen(int fd, const void* buff, size_t n) {
    char* p = (char*)buff;

    size_t nleft, nwritten;
    nleft = n;

    while(nleft > 0) {
        if((nwritten = write(fd, p, nleft)) <= 0) {	// passive close will not happen in this situation
            if(nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        p += nwritten;
    }

    return n;    // must write all content to fd's buffer
}

// returns: -1 on error, otherwise bytes read
ssize_t readline(int fd, void* buff, int maxlen) {
    size_t n, rc;
    char c, *p;

    p = (char*)buff;
    for(n = 1; n < maxlen; n++) {
        // again:
        if((rc = read(fd, &c, 1)) == 1) {
            *p++ = c;
            if(c == '\n')
                break;
        } else if(rc == 0) {	// EOF
            *p = '\0';
            return n-1;
        } else {
            if(errno == EINTR)
                continue;
                // goto again;
            return -1;
        }
    }

    *p = '\0';
    return n;
}

// returns: -1 on error
ssize_t get_http_request(int fd, void* buff, int maxline, int* num_of_line) {
    size_t n, rc, wc = 0, lc = 0;
    char c, *p;
    int flag = 0;

    p = (char*)buff;
    // request head
    for(; wc < MAXHEAD-2 ;) {
        if ((rc = readline(fd, p, maxline)) < 0) {
            DEBUG("readline");
            return -1;
        } else if (rc == 0) {
            return 0;
        }
        if (*p != ' ' && *p != '\t') {  // new head line
            lc += 1;
            if (!strcmp(p, "\r\n")) {
                p += 2;
                flag = 1;
                break;
            }
        } else {
            // previous head line
            char* tmp = p-2; // ends with "\r\n\0", but p point to '\0', so move backward 2 bytes.
            c = *p;
            while(c == ' ' || c == '\t') {
                rc -= 1;
                c = *++p;
            }
            memmove(tmp, p, rc);
        }
        p += rc;
        wc += rc;
    }
    *p = '\0';
    *num_of_line = lc;
    // MAXHEAD LENGTH of GET method request.

    if (flag) {
        return ((void*)p - buff);
    } else {
        return E_URI_OUTRANGE;
    }
}

// TODO: don't only read one byte each time
ssize_t get_http_entity_body(int fd, void* buff, int nbytes) {
    int rc, n = 0;
    char* p = (char*)buff;
    char c;

    while (n < nbytes) {
        if((rc = read(fd, &c, 1)) < 0) {
            if (errno == EINTR) {
                continue;
            }
            DEBUG("get_http_entity_body");
            return -1;
        } else if (rc == 0) {
            return 0;
        }
        *p++ = c;
        n++;
    }
    *p = '\0';
    return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t        n;

    if ((n = read(fd, ptr, nbytes)) == -1) {
        DEBUG("read");
        exit(1);
    }
    return(n);
}

void Write(int fd, void *ptr, size_t nbytes)
{
    if (write(fd, ptr, nbytes) != nbytes) {
        DEBUG("write");
        exit(1);
    }
}

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

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

	return n;	// must write all content to fd's buffer
}

// returns: -1 on error
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
ssize_t get_http_request(int fd, void* buff, int maxline) {
    size_t n, rc;
    char c, *p;

    p = (char*)buff;
    // request head
    for(;;) {
        if ((rc = readline(fd, p, maxline)) < 0) {
            fprintf(stderr, "readline() error, file: %s, line: %d\n", __FILE__, __LINE__);
            return -1;
        } else if (rc == 0) {
            return 0;
        }
        if (!strcmp(p, "\r\n")) {
            p += 2;
            break;
        }
        p += rc;
    }
    *p = '\0';
    return ((void*)p - buff);
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1) {
		fprintf(stderr, "%s\n", "read() error");
		exit(1);
	}
	return(n);
}

void Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes) {
		fprintf(stderr, "%s\n", "write() error");
		exit(1);
	}
}

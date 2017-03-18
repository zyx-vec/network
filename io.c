#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

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
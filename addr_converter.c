#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define IS_DIGIT(ch) (((ch) >= '0') && ((ch) <= '9'))

char* Inet_ntop(const struct sockaddr* sa, socklen_t salen) {
    char port[8];
    static char str[128];
    int p;

    switch(sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in *sock = (struct sockaddr_in*) sa;
            if((inet_ntop(AF_INET, &sock->sin_addr, str, sizeof(str))) == NULL) {
                return NULL;
            }
            if((p = ntohs(sock->sin_port)) != 0) {
                snprintf(port, sizeof(port), ":%d", p);
                strcat(str, port);
                return str;
            }
        }
        case AF_INET6: {
            struct sockaddr_in6* sock = (struct sockaddr_in6*) sa;
            uint8_t* ptr = sock->sin6_addr.s6_addr;
            int c = 0;
            uint8_t byte;
            for(int i = 0; i < salen; i += 2) {
                byte = *ptr++;
                str[c++] = (byte>>4) < 10 ? (byte>>4) + '0' : (byte>>4) - 10 + 'A';
                str[c++] = (byte&0xf) < 10 ? (byte&0xf) + '0' : (byte&0xf) - 10 + 'A';
                byte = *ptr++;
                str[c++] = (byte>>4) < 10 ? (byte>>4) + '0' : (byte>>4) - 10 + 'A';
                str[c++] = (byte&0xf) < 10 ? (byte&0xf) + '0' : (byte&0xf) - 10 + 'A';
                str[c++] = ':';
            }
            str[c] = '\0';
            if((p = ntohs(sock->sin6_port)) != 0) {
                snprintf(port, sizeof(port), "%d", p);
                strcat(str, port);
                return str;
            }
        }
    }

    errno = EAFNOSUPPORT;
    return NULL;
}

// only supports IPv6
int Inet_pton(const char* str, void* addr) {
    const char* p = str;

    uint8_t *ip = addr;
    uint16_t val = 0;
    for(; *p; p++) {
        if(*p != ':') {
            val = val * 16 + ((IS_DIGIT(*p)) ? (*p-'0') : (*p-'A'+10));
        } else {
            *ip++ = (uint8_t)(val >> 8);
            *ip++ = (uint8_t)(val & 0xff);
            val = 0;
        }
    }
    *ip++ = (uint8_t)(val >> 8);
    *ip++ = (uint8_t)(val & 0xff);

    return 0;

    // ptrdiff_t distance = ip - addr;
    // if(distance != 128) {
    //     // TODO: fix ::
    // }
}
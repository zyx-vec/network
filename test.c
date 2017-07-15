#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "sock_ops.h"
#include "addr_converter.h"

#define SA struct sockaddr

int passed, total;

#define EXPECT_BASE(equality,expect,actual,format) do { \
    if(equality) { \
        passed++; total++; \
    } \
    else { \
        total++; \
        fprintf(stderr, "file: %s, line: %d; expect: " format ", actual: " format "\n", __FILE__, __LINE__, (expect), (actual)); \
    } \
} while(0)

#define EXPECT_EQ_INT(expect,actual) EXPECT_BASE((expect)==(actual), expect, actual, "%d")
#define EXPECT_EQ_STRING(expect,actual,alength) \
    EXPECT_BASE(sizeof(expect)-1 == alength && !memcmp(expect, actual, alength), expect, actual, "%s")

void test_sock_ops() {
    struct sockaddr_in s1, s2;
    bzero(&s1, sizeof(s1)), bzero(&s2, sizeof(s2));
    s1.sin_family = AF_INET, s2.sin_family = AF_INET;
    s1.sin_addr.s_addr = htonl(0x1), s2.sin_addr.s_addr = htonl(0x1);
    EXPECT_EQ_INT(0, sock_cmp_addr((struct sockaddr*)&s1, (struct sockaddr*)&s2, sizeof(s1.sin_addr)));
}

void test_addr_converter() {
    struct sockaddr_in s;
    bzero(&s, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(55555);
    s.sin_addr.s_addr = htonl(0xffffffff);
    EXPECT_EQ_STRING("255.255.255.255:55555",
        Inet_ntop((SA*)&s, sizeof(s.sin_addr)), strlen(Inet_ntop((SA*)&s, sizeof(s.sin_addr))));

    struct sockaddr_in6 s6;
    bzero(&s6, sizeof(s6));
    s6.sin6_family = AF_INET6;
    s6.sin6_port = htons(55555);
    //s6.sin6_addr = in6addr_any;
    inet_pton(AF_INET6, "0102:0304:0506:0708:090A:0B0C:0D0E:0F00", &s6.sin6_addr);
    EXPECT_EQ_STRING("0102:0304:0506:0708:090A:0B0C:0D0E:0F00:55555",
        Inet_ntop((SA*)&s6, sizeof(s6.sin6_addr)), strlen(Inet_ntop((SA*)&s6, sizeof(s6.sin6_addr))));

    Inet_pton("0102:0304:0506:0708:090A:0B0C:0D0E:0FFF", &s6.sin6_addr.s6_addr);
    EXPECT_EQ_STRING("0102:0304:0506:0708:090A:0B0C:0D0E:0FFF:55555",
        Inet_ntop((SA*)&s6, sizeof(s6.sin6_addr)), strlen(Inet_ntop((SA*)&s6, sizeof(s6.sin6_addr))));
}

void test() {
    test_sock_ops();
    test_addr_converter();
}


int main() {
    passed = total = 0;
    test();

    fprintf(stderr, "passed/total: %d/%d\n", passed, total);

    return 0;
}
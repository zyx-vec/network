#ifndef ADDR_CONVERTER_H_

#define ADDR_CONVERTER_H_

char* Inet_ntop(const struct sockaddr* sa, socklen_t salen);
int Inet_pton(const char* str, void* addr);

#endif
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

#include "common.h"

ssize_t parse_http_request(char* request, char** lines) {

    char* p = request;
    char c;
    int count = 0;

    lines[count++] = p;
    while ((c = *p++)) {
        if (c == '\r' && *p == '\n' && *(p+1)) {
            *(p-1) = '\0';
            *p = '\0';
            lines[count++] = p+1;
            p++;
        }
    }
    lines[count-1] = NULL;  // The final '\r\n' as delimiter

    // for (int i = 0; i < count; i++) {
    //     printf("line %d: %s\n", i, lines[i]);
    // }

    return 0;
}

ssize_t parse_http_content_length(char** lines) {
    char** pp = lines;
    assert(pp[0] != NULL);
    char* p = pp[0];
    const char text[] = "Content-Length: ";
    int prefix_length = sizeof(text) - 1;

    int result = 0;

    while(*pp++) {
        if (!strncmp(text, p, prefix_length)) {
            result = http_str2int(p+prefix_length);
            return result < 0 ? -1 : result;
        }
        p = *pp;
    }
    return result;
}

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

#include "common.h"

static void print_http_request(struct request_t* request) {
    fprintf(stdout, "method:%surl:%sversion:%s\n",
            request->url[0], request->url[1], request->url[2]);
    int c, n;
    c = 0;
    n = request->head->num_of_head_lines;
    char** lines = request->head->lines;
    for (; c < n; c++) {
        fprintf(stdout, "%s\n", lines[c]);
    }
    return;
}

static ssize_t parse_http_split_lines(char* buff, struct head_t* head) {
    assert(buff != NULL);
    int ret = 0;
    char* p = buff;
    char c;
    
    char** lines = head->lines;
    int num_of_head_lines = head->num_of_head_lines;
    int count = 0;

    while (count < num_of_head_lines) {
        lines[count++] = p;
        char* tmp = strstr(p, "\r\n");
        if (tmp == NULL) {
            ret = -1;
            break;
        }
        *tmp++ = '\0';
        *tmp++ = '\0';
        p = tmp;
    }
    return ret;
}

ssize_t parse_http_request(char* buff, struct request_t* request) {
    assert(buff != NULL && request != NULL);
    int ret = 0;

    char* request_line = buff;
    for (int i = 0; i < 2; i++) {
        char* tmp = strchr(request_line, ' ');
        if (tmp == NULL)
            return -1;
        *tmp++ = '\0';
        request->url[i] = request_line;
        while (*tmp == ' ')
            tmp++;
        request_line = tmp;
    }
    char* tmp = strstr(request_line, "\r\n");
    if (tmp == NULL) {
        return -1;
    } 
    *tmp++ = '\0';
    *tmp++ = '\0';
    request->url[2] = request_line;
    request_line = tmp;

    if (parse_http_split_lines(request_line, request->head) < 0) {
        DEBUG("parse_http_split_lines");
        return -1;
    }

    print_http_request(request);
  
    return ret;
}

ssize_t parse_http_content_length(struct request_t* request) {
    char** pp = request->head->lines;
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

char* parse_http_url_type(char* p) {
    assert(p != NULL);
    char* end = p;

    while(*++end)
        ;
    while(*--end != '.')
        ;
    return end+1;
}

ssize_t parse_http_request_filename(char* filename, struct request_t* request) {
    char* p = request->url[1];
    // TODO: add parameter support for GET method.
    if (*p == '/' && *(p+1) == '\0') {
        strcat(filename, "index.html");
        return 1;
    } else if (!strcmp(p, "/home")) {
        strcat(filename, "homepage.html");
        return 1;
    }
    
    strcat(filename, ++p);
    return 1;
}

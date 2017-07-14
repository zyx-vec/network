#ifndef _PARSE_H
#define _PARSE_H

ssize_t parse_http_request(char* request, char** p);

ssize_t parse_http_content_length(char** p);

#endif

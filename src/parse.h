#ifndef _PARSE_H
#define _PARSE_H

ssize_t parse_http_request(char* buff, struct request_t* request);

ssize_t parse_http_content_length(struct request_t* request);

char* parse_http_url_type(char* p);

ssize_t parse_http_request_filename(char* filename, struct request_t* request);

ssize_t parse_http_is_keep_alive(struct request_t* request);

#endif

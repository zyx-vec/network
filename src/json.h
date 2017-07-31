#ifndef __JSON_H
#define __JSON_H

#include <stddef.h>

typedef enum {
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type;

typedef struct json_value json_value;
typedef struct json_member json_member;

struct json_value {
    union {
        struct { json_member* m; size_t size; } o;  /* object */
        struct { json_value* e; size_t size; } a;   /* array */
        struct { char* s; size_t len; } s;          /* string: null-terminated string, string length */
        double n;                                   /* number */
    } u;
    json_type type;
};

struct json_member {
    char* k;
    size_t ksize;   /* member key string, key string length */
    json_value v;   /* member value */
};

enum {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_TOO_BIG,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_INVALID_STRING_CHAR,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

#define json_init(v) do { (v)->type = JSON_NULL; } while(0)

int json_parse(json_value* v, const char* json);

void json_free(json_value* v);

json_type json_get_type(const json_value* v);

#define json_set_null(v) json_free(v)

int json_get_boolean(const json_value* v);
void json_set_boolean(json_value* v, int b);

double json_get_number(const json_value* v);
void json_set_number(json_value* v, double n);

const char* json_get_string(const json_value* v);
size_t json_get_string_length(const json_value* v);
void json_set_string(json_value* v, const char* s, size_t len);

size_t json_get_array_size(const json_value* v);
json_value* json_get_array_element(const json_value* v, size_t index);

size_t json_get_object_size(const json_value* v);
const char* json_get_object_key(const json_value* v, size_t index);
size_t json_get_object_key_length(const json_value* v, size_t index);
json_value* json_get_object_value(const json_value* v, size_t index);

#endif


#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */
#include <stdio.h>
#include "json.h"

#ifndef PARSE_STACK_INIT_SIZE
#define PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)         do { *(char*)context_push(c, sizeof(char)) = (ch); } while(0)
#define PUTV(c, v)          do { *(json_value*)context_push(c, sizeof(json_value)) = (v); } while(0)
#define PUTM(c, m)          do { *(json_member*)context_push(c, sizeof(json_member)) = (m); } while(0)

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
} context;

static void* context_push(context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* context_pop(context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static void parse_whitespace(context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int parse_literal(context* c, json_value* v, const char* literal, json_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1])
            return PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return PARSE_OK;
}

static int parse_number(context* c, json_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return PARSE_NUMBER_TOO_BIG;
    v->type = JSON_NUMBER;
    c->json = p;
    return PARSE_OK;
}

static const char* parse_hex4(const char* p, unsigned* u) {
    unsigned short H;
    size_t i;
    char ch;
    H = 0;
    for(i = 0; i < 4; i++, p++) {
        ch = *p;
        if(ISDIGIT(ch)) {
            H |= (ch-'0') << ((3-i)*4);
        }
        else if((ch|0x20)>='a' && (ch|0x20)<='f') {
            H |= ((ch|0x20)-'a'+10) << ((3-i)*4);
        }
        else {
            return NULL;    /* LEPT_PARSE_INVALID_UNICODE_HEX */
        }
    }
    *u |= H;
    return p;
}


static void encode_utf8(context* c, unsigned u) {
    unsigned code_point;

    if(u <= 0xFFFF) {
        code_point = u;
    } else {
        code_point = 0x10000 + (((u>>16) - 0xD800)<<10) + ((u&0xFFFF) - 0xDC00);
    }

    if(code_point <= 0x007F) {
        /* one bytes: 7 */
        PUTC(c, code_point);
    }
    else if(code_point <= 0x07FF) {
        /* two bytes: 5 + 6 */
        PUTC(c, 0xC0 | ((code_point >>  6) & 0xFF));
        PUTC(c, 0x80 | ( code_point        & 0x3F));
    }
    else if(code_point <= 0xFFFF) {
        /* three bytes: 4 + 6 + 6 */
        PUTC(c, 0xE0 | ((code_point >> 12) & 0xFF));
        PUTC(c, 0x80 | ((code_point >>  6) & 0x3F));
        PUTC(c, 0x80 | ( code_point        & 0x3F));
    }
    else {
        /* four bytes： 3 + 6 + 6 + 6 */
        PUTC(c, 0xF0 | ((code_point >> 18) & 0xFF));
        PUTC(c, 0x80 | ((code_point >> 12) & 0x3F));
        PUTC(c, 0x80 | ((code_point >>  6) & 0x3F));
        PUTC(c, 0x80 | ( code_point        & 0x3F));
    }
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

static int parse_string_raw(context* c, char** str, size_t* len) {
    size_t head = c->top;
    unsigned u;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    u = 0;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                *len = c->top - head;
                *str = context_pop(c, *len);
                c->json = p;
                return PARSE_OK;
            case '\\':
                switch (*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if (!(p = parse_hex4(p, &u)))
                            STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                        /* surrogate handling */
                        if(u >= 0xD800 && u <= 0xD8FF) {
                            u <<= 16;
                            if(p[0] != '\\' || p[1] != 'u')
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            p += 2;
                            if(!(p = parse_hex4(p, &u)))
                                STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                            if(!((u&0xffff)>=0xDC00 && (u&0xffff)<=0xDFFF))
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                        }
                        else if(u > 0xD8FF)
                            STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                        encode_utf8(c, u);
                        u = 0;
                        break;
                    default:
                        STRING_ERROR(PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                STRING_ERROR(PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20)
                    STRING_ERROR(PARSE_INVALID_STRING_CHAR);
                PUTC(c, ch);
        }
    }
}

static int parse_string(context* c, json_value* v) {
    int ret;
    char* s;
    size_t len;
    if((ret = parse_string_raw(c, &s, &len)) == PARSE_OK) {
        json_set_string(v, s, len);
    }
    return ret;
}

static int parse_value(context* c, json_value* v);

static int parse_array(context* c, json_value* v) {
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    
    parse_whitespace(c);
    if(*c->json == ']') {
        c->json++;
        v->type = JSON_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return PARSE_OK;
    }

    for(;;) {
        json_value val;
        json_init(&val);
        if((ret = parse_value(c, &val)) != PARSE_OK) {
            context_pop(c, c->top);
            return ret;
        }
        PUTV(c, val);
        size++;
        parse_whitespace(c);
        if(*c->json == ',') {
            c->json++;
            parse_whitespace(c);   /* next parse loop */
            if(*c->json == ']') {
                context_pop(c, c->top);
                return PARSE_INVALID_VALUE;
            }
        } else if(*c->json == ']') {
            c->json++;
            v->type = JSON_ARRAY;
            v->u.a.size = size;
            size *= sizeof(json_value);
            memcpy(v->u.a.e = (json_value*)malloc(size), context_pop(c, size), size);
            return PARSE_OK;
        } else {
            context_pop(c, c->top);
            return PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }

    return PARSE_OK;
}

static int parse_object(context* c, json_value* v) {
    size_t size;
    int ret;
    json_member m;
    EXPECT(c, '{');
    parse_whitespace(c);

    if(*c->json == '}') {
        c->json++;
        v->type = JSON_OBJECT;
        v->u.o.size = 0;
        v->u.o.m = NULL;
        return PARSE_OK;
    }

    size = 0;
    for(;;) {
        char *t;
        m.k = NULL;
        json_init(&m.v);

        if(*c->json != '\"') {
            ret = PARSE_MISS_KEY;
            break;
        }
        if((ret = parse_string_raw(c, &t, &m.ksize)) != PARSE_OK) {
            break;
        }
        m.k = (char*)malloc(m.ksize);
        memcpy(m.k, t, m.ksize);
        parse_whitespace(c);
        if(*c->json == ':') {
            c->json++;
            parse_whitespace(c);
        } else {
            free(m.k);
            ret = PARSE_MISS_COLON;
            break;
        }

        if((ret = parse_value(c, &m.v)) != PARSE_OK) {
            break;
        }
        size++;
        PUTM(c, m);

        parse_whitespace(c);
        if(*c->json == ',') {
            c->json++;
            parse_whitespace(c);
        } else if(*c->json == '}') {
            c->json++;
            v->type = JSON_OBJECT;
            v->u.o.size = size;
            size *= sizeof(json_member);
            memcpy((v->u.o.m = (json_member*)malloc(size)), context_pop(c, size), size);
            return PARSE_OK;
        } else {
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    context_pop(c, c->top);
    return ret;
}


static int parse_value(context* c, json_value* v) {
    switch (*c->json) {
        case 't':  return parse_literal(c, v, "true", JSON_TRUE);
        case 'f':  return parse_literal(c, v, "false", JSON_FALSE);
        case 'n':  return parse_literal(c, v, "null", JSON_NULL);
        default:   return parse_number(c, v);
        case '"':  return parse_string(c, v);  /* same as '\"' */
        case '[': return parse_array(c, v);
        case '{': return parse_object(c, v);
        case '\0': return PARSE_EXPECT_VALUE;
    }
}

int parse(json_value* v, const char* json) {
    context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    json_init(v);
    parse_whitespace(&c);
    if ((ret = parse_value(&c, v)) == PARSE_OK) {
        parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = JSON_NULL;
            ret = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}


void json_free(json_value* v) {
    assert(v != NULL);
    size_t i = 0;
    if (v->type == JSON_STRING)
        free(v->u.s.s);
    if (v->type == JSON_ARRAY) {
        for(i = 0; i < v->u.a.size; i++) {
            json_free(&v->u.a.e[i]);    /* free it recurisively */
        }
        free(v->u.a.e);
    }
    v->type = JSON_NULL;
}

json_type json_get_type(const json_value* v) {
    assert(v != NULL);
    return v->type;
}

/* boolean access functions */
int json_get_boolean(const json_value* v) {
    assert(v != NULL && (v->type == JSON_TRUE || v->type == JSON_FALSE));
    return v->type == JSON_TRUE;
}

void json_set_boolean(json_value* v, int b) {
    json_free(v);
    v->type = b ? JSON_TRUE : JSON_FALSE;
}

/* number access functions */
double json_get_number(const json_value* v) {
    assert(v != NULL && v->type == JSON_NUMBER);
    return v->u.n;
}

void json_set_number(json_value* v, double n) {
    json_free(v);
    v->u.n = n;
    v->type = JSON_NUMBER;
}

/* string access functions */
const char* json_get_string(const json_value* v) {
    assert(v != NULL && v->type == JSON_STRING);
    return v->u.s.s;
}

size_t json_get_string_length(const json_value* v) {
    assert(v != NULL && v->type == JSON_STRING);
    return v->u.s.len;
}

void json_set_string(json_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    json_free(v);
    v->u.s.s = (char*)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = JSON_STRING;
}

/* array access functions */
size_t json_get_array_size(const json_value* v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    return v->u.a.size;
}

json_value* json_get_array_element(const json_value* v, size_t index) {
    assert(v != NULL && v->type == JSON_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

/* object access functions */
size_t json_get_object_size(const json_value* v) {
    assert(v != NULL && v->type == JSON_OBJECT);
    return v->u.o.size;
}

const char* json_get_object_key(const json_value* v, size_t index) {
    assert(v != NULL && v->type == JSON_OBJECT);
    assert(index < json_get_object_size(v));
    return v->u.o.m[index].k;
}

size_t json_get_object_key_length(const json_value* v, size_t index) {
    assert(v != NULL && v->type == JSON_OBJECT);
    assert(index < json_get_object_size(v));
    return v->u.o.m[index].ksize;
}

json_value* json_get_object_value(const json_value* v, size_t index) {
    assert(v != NULL && v->type == JSON_OBJECT);
    assert(index < json_get_object_size(v));
    return &v->u.o.m[index].v;
}


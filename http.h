#ifndef HTTP_H
#include <sys/types.h>

struct http_response {
    char        *h_buf;
    ssize_t     h_size;
    char        *b_buf;
    ssize_t     b_size;
};

struct http_response * http_request(const char *, short, const char *, const char *, size_t);
void http_free(struct http_response *);

#endif

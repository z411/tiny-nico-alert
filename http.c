/* Copyright (c) 2016 z411, see LICENSE for details */

#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "net.h"

void PANIC(char *msg);
#define PANIC(msg) { perror(msg); exit(-1); }

struct http_response *
http_request(const char *host, short port, const char *path, const char *post, size_t post_sz)
{
    struct conn *c;

    c = conn_connect(host, port, (port == 443 ? 1 : 0));

    char *req;
    int req_sz;

    if (post == NULL) {
        req_sz = asprintf(&req,
                "GET %s HTTP/1.0\r\n"
                "Host: %s\r\n"
                "\r\n",
                path, host);
    } else {
        req_sz = asprintf(&req,
                "POST %s HTTP/1.0\r\n"
                "Host: %s\r\n"
                "Content-Length: %zu\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "\r\n",
                path, host, post_sz);
    }

    c->write(c, req, req_sz);
    free(req);

    if (post != NULL) {
        c->write(c, post, post_sz);
    }

    struct http_response *r;
    r = calloc(1, sizeof(struct http_response));

    /* Skip headers */
    char buf[BUFSIZ];
    size_t r_size;

    char *newbuf;
    char *h_end;

    do {
        /* Read bytes */
        r_size = c->read(c, buf, sizeof(buf));

        /* Look for the end of headers */
        h_end = strstr(buf, "\r\n\r\n");
    } while(h_end == NULL && r_size == sizeof(buf));

    /* Read body */
    h_end += 4;
    r->b_size = (buf + r_size) - h_end;
    r->b_buf = malloc(r->b_size);
    memcpy(r->b_buf, h_end, r->b_size);

    do {
        // Read remaining bytes
        r_size = c->read(c, buf, sizeof(buf));
        if (r_size < 0)
            return NULL; // ???
        if (r_size == 0)
            break; // We're done here, no more data

        // Realloc size of body buffer
        newbuf = realloc(r->b_buf, r->b_size + r_size);
        if (newbuf == NULL)
            return NULL; // realloc failure
        r->b_buf = newbuf;

        // Copy new contents into body buffer
        memcpy(r->b_buf + r->b_size, buf, r_size);
        r->b_size += r_size;
    } while (1);

    conn_close(c);

    return r;
}

void
http_free(struct http_response *r)
{
    free(r->h_buf);
    free(r->b_buf);
    free(r);
}


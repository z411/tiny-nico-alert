/* Copyright (c) 2016 z411, see LICENSE for details */

#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <errno.h>

void PANIC(char *msg);
#define PANIC(msg) { perror(msg); exit(-1); }

static struct tls_config *tlscfg = NULL;

ssize_t
conn_read(const struct conn *c, char *buf, size_t size)
{
    ssize_t tot = read(c->fd, buf, size);
    if(tot < 1) return 0; else return tot;
}

ssize_t
conn_tls_write(const struct conn *c, const void *buf, size_t size)
{
    return tls_write(c->ctx, buf, size);
}

ssize_t
conn_tls_read(const struct conn *c, char *buf, size_t size)
{
    ssize_t tot = tls_read(c->ctx, buf, size);
    if(tot < 1) return 0;  else return tot;
}

ssize_t
conn_write(const struct conn *c, const void *buf, size_t size)
{
    return write(c->fd, buf, size);
}

void
conn_tls_init()
{
    if (tlscfg == NULL) {
        tls_init();
        tlscfg = tls_config_new();
    }
}

void
conn_tls_uninit()
{
    if (tlscfg != NULL) {
        tls_config_free(tlscfg);
    }
}

struct conn *
conn_connect(const char *host, short port, int secure)
{
    struct conn *c;

    int fd;
    struct sockaddr_in dest;
    struct hostent *he;

    he = gethostbyname(host);

    if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        PANIC("Socket");

    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    //dest.sin_addr.s_addr = inet_addr(host);
    memcpy(&dest.sin_addr.s_addr, he->h_addr, he->h_length);

    if( connect(fd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
        PANIC("Connect");

    /* Allocate connector */
    c = calloc(1, sizeof(struct conn));
    if (c == NULL) PANIC("conn");

    c->fd = fd;

    if (!secure) {
        c->read = conn_read;
        c->write = conn_write;
        return c;
    }

    c->read = conn_tls_read;
    c->write = conn_tls_write;

    conn_tls_init();

    if ((c->ctx = tls_client()) == NULL) {
        PANIC("SSL");
    } else if (tls_configure(c->ctx, tlscfg) == -1) {
        PANIC("ssl_configure");
    }

    if (tls_connect_socket(c->ctx, c->fd, host) != 0) {
        PANIC("SSL connect");
    }

    return c;
}

void
conn_close(struct conn *c)
{
    if (c->ctx != NULL) {
        tls_close(c->ctx);
        tls_free(c->ctx);
    } else {
        if (close(c->fd) == -1)
            PANIC("Close");
    }

    c->fd = -1;
    free(c);
}


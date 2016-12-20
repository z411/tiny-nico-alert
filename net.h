#ifndef NET_H
#include <sys/types.h>
#include <tls.h>

struct conn;

typedef ssize_t (*readfunc) (const struct conn *, char *, size_t);
typedef ssize_t (*writefunc) (const struct conn *, const void *, size_t);

struct conn {
    int         fd;
    struct tls  *ctx;
    readfunc    read;
    writefunc   write;
};

ssize_t conn_read(const struct conn *, char *, size_t);
ssize_t conn_write(const struct conn *c, const void *, size_t);
ssize_t conn_tls_read(const struct conn *, char *, size_t);
ssize_t conn_tls_write(const struct conn *c, const void *, size_t);
void conn_tls_init();
void conn_tls_uninit();
struct conn * conn_connect(const char *, short, int);
void conn_close(struct conn *);

#endif

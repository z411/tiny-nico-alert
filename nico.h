#ifndef NICO_H
#include <sys/types.h>
#include "khash.h"

KHASH_SET_INIT_STR(co_set)

struct stream_info {
    char *title;
    char *thumb;
};

struct server_info {
    char *host;
    short port;
    char *thread;

    char *username;
    khash_t(co_set) *co;
    int co_sz;
};

void alert(const char *);
struct server_info * info_parse(char *, size_t);
struct stream_info * get_stream_info(const char *);

void free_info(struct server_info *);
void free_stream(struct stream_info *);

char * parse_tag(char *, size_t, char *);
void got_chat(struct server_info *, char *);

int alert_connect(struct server_info *);
struct server_info * nico_get_info(const char *, const char *);

#endif


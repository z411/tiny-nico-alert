/* Copyright (c) 2016 z411, see LICENSE for details */

#include "nico.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include "net.h"
#include "http.h"

void PANIC(char *msg);
#define PANIC(msg) { perror(msg); exit(-1); }

void
alert(const char *live)
{
    printf("ALERT !!! http://live.nicovideo.jp/watch/lv%s\n", live);
    if (config.cmd != NULL) {
        // Get stream information
        struct stream_info *stream;
        if ((stream = get_stream_info(live)) != NULL) {
            setenv("TITLE", stream->title, 1);
            setenv("THUMB", stream->thumb, 1);
            free_stream(stream);
        } else {
            puts("Warn: Stream info not available");
        }

        char *url;
        asprintf(&url, "http://live.nicovideo.jp/watch/lv%s", live);
        setenv("URL", url, 1);
        free(url);

        // Execute our command
        run_cmd();
    }
}

struct stream_info *
get_stream_info(const char *live)
{
    struct stream_info *stream;
    struct http_response *r;

    char *get;
    asprintf(&get, "http://live.nicovideo.jp/api/getstreaminfo/lv%s", live);
    printf("get: %s\n", get);

    r = http_request("live.nicovideo.jp", 80, get, NULL, 0);
    free(get);

    if( strstr(r->b_buf, "<error>") == NULL ) {
        stream = calloc(1, sizeof(struct stream_info));
        stream->title = parse_tag("<title>", 7, r->b_buf);
        stream->thumb = parse_tag("<thumbnail>", 11, r->b_buf);
    } else {
        stream = NULL;
    }

    http_free(r);

    return stream;
}

void
free_stream(struct stream_info *stream)
{
    free(stream->title);
    free(stream->thumb);
    free(stream);
}

char *
parse_tag(char * tagname, size_t tagsize, char *body)
{
    char *p, *in_p;
    p = strstr(body, tagname);
    if(p != NULL) {
        p += tagsize;
        in_p = strchr(p, '<');
    } else
        return NULL;

    ssize_t sz = in_p - p;
    char *res = malloc(sz * sizeof(char));
    strncpy(res, p, sz);
    res[sz] = '\0';

    return res;
}

struct server_info *
info_parse(char *body, size_t size)
{
    char *p = body;
    char *in_p;

    /* Prepare info struct */
    struct server_info *info;
    info = calloc(1, sizeof(struct server_info));

    /* Get user information */
    info->username = parse_tag("<user_name>", 11, p);

    /* Initialize community set */
    info->co = kh_init(co_set);

    /* Search for communities */
    do {
        if ((p = strstr(p, "<community_id>")) == NULL) {
            break;
        }

        p += 14;
        in_p = strchr(p, '<');
        *in_p++ = '\0';

        /* Add community to our hash set */
        char *this_co;
        this_co = strdup(p);

        int result;
        kh_put(co_set, info->co, this_co, &result);
    } while((p = in_p) < (body + size));

    /* Total communities */
    info->co_sz = kh_size(info->co);

    /* Get server info */
    info->host = parse_tag("<addr>", 6, in_p);
    info->thread = parse_tag("<thread>", 8, in_p);

    char *tmp = parse_tag("<port>", 6, in_p);
    info->port = atoi(tmp);
    free(tmp);

    return info;
}

void
free_info(struct server_info *info)
{
    free(info->username);
    free(info->host);
    free(info->thread);
    kh_destroy(co_set, info->co);
    free(info);
}

void
got_chat(struct server_info *info, char *str)
{
    char *p = str;
    char *in_p;

    char *live, *co, *user;

    do {
        if((p = strstr(p, "<chat")) == NULL) {
            break;
        }
        p = strchr(p, '>');
        p++;

        in_p = strchr(p, ',');
        *in_p++ = '\0';
        live = p;

        p = in_p;

        in_p = strchr(p, ',');
        *in_p++ = '\0';
        co = p;

        p = in_p;

        in_p = strchr(p, '<');
        *in_p++ = '\0';
        user = p;

        p = in_p;

        /* Check if we need to alert about this */
        //printf("check %s ... ", co);

        khint_t k;
        k = kh_get(co_set, info->co, co);
        if (k != kh_end(info->co))
            alert(live);
        //else
        //    printf("nothing\n");
    } while(1);
}

void fix_chat(char *buf, size_t size)
{
    char *p = buf;
    for(; size; p++, size--)
        if(*p == '\0') *p = '!';
    *p = '\0';
}

int
alert_connect(struct server_info *info)
{
    puts("アラートサーバーに接続しています...");

    struct conn *c;
    char *req;

    c = conn_connect(info->host, info->port, 0);
    asprintf(&req, "<thread thread=\"%s\" version=\"20061206\" res_from=\"-1\"/>", info->thread);

    puts("接続成功。起動中...");
    c->write(c, req, strlen(req)+1);
    free(req);

    puts("受信中.");

    char buf[BUFSIZ];
    size_t bytes;

    char msg[8192];
    char *msg_p;
    size_t msg_sz;

    //char buf2[] = "<chat thread=\"1000000005\" no=\"30488540\" date=\"1482152752\" date_usec=\"779280\" user_id=\"394\" premium=\"2\">285283737,co3268,25140663</chat> <chat thread=\"1000000005\" no=\"30488541\" date=\"1482152752\" date_usec=\"809269\" user_id=\"394\" premium=\"2\">285283746,co471956,7781544</chat>";

    do {
        msg_p = &msg[0];
        msg_sz = 0;

        do {
            bytes = c->read(c, buf, sizeof(buf));

            memcpy(msg_p + msg_sz, buf, bytes);
            msg_sz += bytes;

            if(msg_sz >= 8192) {
                printf("Buffer overflow. Exit.\n");
                return 0;
            }
        } while(bytes == sizeof(buf) || buf[bytes-1] != '\0');


        /*if (buf[bytes-1] != '!') {
            printf("WARN: SHOULD GET MORE!!!\n");
            bytes = c->read(c, buf, sizeof(buf));
            printf("Done.\n");
            fix_chat(buf, bytes);
            printf("Contents: %s\n", buf);

            return 0;
        }*/

        //strcpy(msg, buf2); // DEBUG
        fix_chat(msg, msg_sz);
        got_chat(info, msg);
    } while (1);

    conn_close(c);

    return 1;
}

struct server_info *
nico_get_info(const char *mail, const char *pass)
{
    /* Login and get nico alert ticket */
    puts("ログイン中...");

    struct http_response *r;
    struct server_info *info;

    char *ticket, *err;

    char *post;
    int post_size;

    post_size = asprintf(&post, "mail=%s&password=%s", mail, pass);

    r = http_request("secure.nicovideo.jp", 443, "/secure/login?site=nicolive_antenna", post, post_size);
    free(post);

    if ((ticket = parse_tag("<ticket>", 8, r->b_buf)) == NULL) {
        if ((err = parse_tag("<description>", 13, r->b_buf)) != NULL) {
            printf("Error: %s\n", err);
            free(err);
        } else {
            printf("Unknown error.\n");
        }

        return NULL;
    }

    http_free(r);
    conn_tls_uninit(); // we're not using SSL anymore

    /* Send ticket and get server and community information */
    puts("アカウント情報を取り得中...");

    post_size = asprintf(&post, "ticket=%s", ticket);
    free(ticket);

    r = http_request("live.nicovideo.jp", 80, "/api/getalertstatus", post, post_size);
    free(post);

    info = info_parse(r->b_buf, r->b_size);
    http_free(r);

    printf("ようこそ、%sさん。フォローしているコミュニティーは只今%d.\n", info->username, info->co_sz);

    return info;
}


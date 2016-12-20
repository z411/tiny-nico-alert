/* Copyright (c) 2016 z411, see LICENSE for details */

#include "core.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXLINE 100

struct Config config = { NULL, NULL, NULL };

void
run_cmd(const char * url)
{
    system(config.cmd);
}

void
cfg_apply_bool(int * dest, char * val)
{
    *dest = (!strcmp(val, "yes")) ? 1 : 0;
}

void
cfg_apply_int(int * dest, char * val)
{
    *dest = (int)strtol(val, NULL, 10);
}

void
cfg_apply_str(char ** dest, char * key, char * val)
{
    if (*dest == NULL) {
        *dest = malloc((strlen(val)+1)*sizeof(*dest));
        if (dest != NULL) strcpy(*dest, val);
    } else
        printf("config: Duplicate option, ignoring: %s\n", key);
}

void
cfg_apply(char * key, char * val)
{
    if (!strcmp(key, "mail")) {
        cfg_apply_str(&config.mail, "mail", val);
    } else if (!strcmp(key, "pass")) {
        cfg_apply_str(&config.pass, "pass", val);
    } else if (!strcmp(key, "cmd")) {
        cfg_apply_str(&config.cmd, "cmd", val);
    } else {
        printf("config: Invalid option, ignoring: %s\n", key);
    }
}

void
parse_config(char * filename)
{
    FILE *fp;
    char line[MAXLINE];

    if ((fp = fopen(filename, "r")) != NULL) {
        char * key;
        char * val;

        while (fgets(line, MAXLINE, fp)) {
            /* Ignore comment */
            if(line[0] == '#') continue;

            /* Remove endline */
            int ln = strlen(line) - 1;
            if(line[ln] == '\n')
                line[ln--] = 0;

	    /* Remove CR */
	    if(line[ln] == '\r')
                line[ln--] = 0;

	    /* Ignore empty lines */
            if(ln<0) continue;

            /* Parse config line */
            key = strtok(line, "=");
            val = strtok(NULL, "=");
            if (val != NULL) {
                strip(key);
                strip(val);
                cfg_apply(key, val);
            } else {
                printf("config: Invalid line, ignoring: %s\n", line);
            }
        }

        fclose(fp);
    } else {
        perror("Error reading configuration file");
        exit(-1);
    }
}

void
cleanup()
{
    free(config.cmd);
    free(config.mail);
    free(config.pass);
}

void
strip(char * str)
{
    /* Strips whitespace from beginning and end */
    if (*str) {
        char *p, *q;
        for (p = q = str; *p == ' '; p++);
        while (*p) *q++ = *p++;
        for (; *(q-1) == ' '; q--);
        *q = '\0';
    }
}

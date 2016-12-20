#ifndef CORE_H
#define CORE_H
#define _VERSION "0.1"

#include <stddef.h>

struct Config
{
    char * mail;
    char * pass;
    char * cmd;
};

extern struct Config config;

void run_cmd();
void cfg_apply_bool(int * dest, char * val);
void cfg_apply_int(int * dest, char * val);
void cfg_apply_str(char ** dest, char * key, char * val);
void cfg_apply(char * key, char * val);
void parse_config();
void cleanup();

void strip(char * str);

#endif

/* Copyright (c) 2016 z411, see LICENSE for details */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include "nico.h"

#include <mcheck.h>

int
main()
{
    puts("nico-watcher ニコ生アラート");
    puts("---------------------------");

    puts("コンフィグを読込中...");
    parse_config("config");

    if (config.mail == NULL || config.pass == NULL) {
        puts("Please include your credentials in config file.");
        return 1;
    }

    struct server_info *info;

    info = nico_get_info(config.mail, config.pass);

    if (info != NULL) {
        alert_connect(info);
        free_info(info);
    }

    return 0;
}

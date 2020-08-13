#ifndef WATCH_H
#define WATCH_H

#include "ping.h"

#define COMMAND_BUFFER_SIZE 1024

typedef struct watch_options {
    char command[COMMAND_BUFFER_SIZE];
    double interval;
    int show_title;
    int precise_timekeeping;
} watch_options;

int start_watch(struct ping_setup_data *pingSetupData, watch_options *watch_args);

#endif
#include <stdio.h>
#include <time.h>

#include "../config.h"
#include "logger.h"

static FILE *server_log = NULL;

static sid session_id = 0;

static time_t cur_time;

void init_logger() {
    server_log = fopen(log_filename, "ab+");

    time(&cur_time);
    fprintf(server_log, "%lld    start\n", (long long)cur_time);
    fflush(server_log);
}

void add_log(sid ses_id, int action) {
    if (server_log == NULL) {
        return;
    }

    time(&cur_time);
    fprintf(server_log, "%lld    %d    %llu\n", (long long)cur_time, action,
            ses_id);
    fflush(server_log);
}

void save_log() {
    time(&cur_time);
    fprintf(server_log, "%lld    stop\n", (long long)cur_time);
    fclose(server_log);
}

sid get_id() {
    session_id++;
    return session_id - 1;
}

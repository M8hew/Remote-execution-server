#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../config.h"
#include "obj_list.h"

typedef struct statistics {
    long long lt_creat;
    long long lt_end;
    long long started;
    long long ended;
    long long last_start;
} statistics;

static statistics st = {0};

static time_t start_time;
static int T;

void actualize_info() {
    time_t tmp_time;
    time(&tmp_time);
    long long cnt_start_time = (long long)tmp_time - T;
    if (cnt_start_time < (long long)start_time) {
        cnt_start_time = (long long)start_time;
    }

    while (1) {
        Node *tail = get_tail();
        if (tail->next == NULL) {
            break;
        }
        if (tail->time >= cnt_start_time) {
            break;
        }
        delete_tail();
    }

    long long local_create = 0;
    long long local_end = 0;
    for (Node *ptr = get_tail(); ptr->next != NULL; ptr = ptr->next) {
        if (ptr->type == CREATED) {
            local_create++;
        } else {
            local_end++;
        }
    }
    st.lt_creat = local_create;
    st.lt_end = local_end;
}

void print_data() {
    time_t cur;
    time(&cur);

    printf("\n");
    if (st.last_start == 0) {
        printf("log about starting server, wasn't added\n");
    } else {
        printf("For last T sec(or sec since start)\n");
        printf("ses created:\t%lld\n", st.lt_creat);
        printf("ses end:\t%lld\n", st.lt_end);
        printf("\n");

        printf("Since last server start\n");
        printf("sec passed:\t%lld\n", (long long)cur - st.last_start);
        printf("ses ended:\t%lld\n", st.ended);
        printf("ses active:\t%lld\n", st.started - st.ended);
    }
    fflush(stdout);
}

void handler(int sig) {
    (void)sig;
    print_data();
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Not enough args were provided\n");
        return 1;
    }
    time(&start_time);
    T = atoi(argv[1]);

    init_list();

    signal(SIGINT, handler);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    FILE *fp = fopen(log_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "log was'nt found -> creating empty log\n");
        fflush(stderr);
        fp = fopen(log_filename, "w+");
    }
    if (fp == NULL) {
        fprintf(stderr, "Problem finding/creating logfile\n");
        return 1;
    }

    long long log_time;
    int action_type;
    long long unsigned session_id;
    do {
        sigprocmask(SIG_BLOCK, &mask, NULL);

        int red;
        do {
            red = fscanf(fp, "%lld\t%d\t%llu", &log_time, &action_type,
                         &session_id);
            if (red != 3) {
                break;
            }

            if (action_type == SERV_LOG) {
                if (session_id == 0) {
                    st.last_start = log_time;
                    st.started = 0;
                    st.ended = 0;
                }
            } else if (action_type == SES_CREATE) {
                st.started++;
                add_node(log_time, CREATED);
            } else if (action_type == SES_ERROR) {
                st.ended++;
                add_node(log_time, ENDED);
            } else if (action_type == SES_END) {
                st.ended++;
                add_node(log_time, ENDED);
            }
        } while (1);
        actualize_info();

        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        sleep(1);
    } while (1);

    fclose(fp);
    return 0;
}
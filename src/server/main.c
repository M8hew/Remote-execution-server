#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../config.h"

enum {
    ARG_SZ = 1024,
};

int create_listener(char *service) {
    struct addrinfo *res = NULL;
    struct addrinfo hint = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };

    int gai_err = getaddrinfo(NULL, service, &hint, &res);
    if (gai_err != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(gai_err));
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            continue;
        }

        int one = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        if (listen(sock, SOMAXCONN) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

// port
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough args were provided\n");
        return 1;
    }

    if (daemon(1, 0) != 0) {
        fprintf(stderr, "Error occured becoming a daemon\n");
        return 1;
    }

    int sock = create_listener(argv[1]);
    if (sock < 0) {
        return 1;
    }

    int connection;
    while (connection = accept(sock, NULL, NULL)) {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Error occured while creating proxy-process\n");
            return 1;
        }
        if (pid == 0) {
            // proxy-process
            dup2(connection, STDIN_FILENO);
            dup2(connection, STDOUT_FILENO);
            dup2(connection, STDERR_FILENO);
            close(connection);

            // take type of command and number of args
            int type, arg_num;
            scanf("%d%d", &type, &arg_num);

            switch (type) {
            case SPAWN: {
                char **arg_vec = calloc(arg_num + 1, sizeof(*arg_vec));
                for (int i = 0; i < arg_num; ++i) {
                    char *arg = calloc(1, ARG_SZ * sizeof(*arg));
                    scanf("%s", arg);
                    arg_vec[i] = arg;
                }
                execvp(arg_vec[0], arg_vec);
                break;
            }
            }
            fprintf(stderr, "error appered runnning comand\n");
            break;
        }
        close(connection);
        waitpid(pid, NULL, 0);
    }

    close(sock);
    return 0;
}
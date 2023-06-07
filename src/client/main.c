#include <inttypes.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../config.h"

enum {
    ADDR_MIN_LEN = 3,
    BUF_SZ = 1024,
};

int create_connection(char *node, char *service) {
    struct addrinfo *res = NULL;
    struct addrinfo hint = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    int gai_err = getaddrinfo(node, service, &hint, &res);
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
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

char *prep_addr(char *addr) {
    if (strlen(addr) < ADDR_MIN_LEN) {
        return NULL;
    }

    char *end = addr + strlen(addr);

    for (char *ptr = end - 1; ptr != addr; --ptr) {
        if (*ptr == ':') {
            *ptr = '\0';
            return ptr + 1;
        }
    }
    return NULL;
}

void *thread_io(void *fd_ptr) {
    int sockfd = *(int *)fd_ptr;

    while (1) {
        char buf[BUF_SZ];
        ssize_t red = read(sockfd, buf, BUF_SZ);
        if (red == 0) {
            shutdown(sockfd, SHUT_RD);
            break;
        }
        write(STDOUT_FILENO, buf, red);
    }

    return NULL;
}

// host service
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "not enough arguments were given\n");
        return 1;
    }

    char *addr = argv[1];
    char *port = prep_addr(argv[1]);
    if (port == NULL) {
        fprintf(stderr, "incorrect address form\n");
        return 1;
    }

    int sockfd = create_connection(addr, port);
    if (sockfd == -1) {
        fprintf(stderr, "error creating connection\n");
        return 1;
    }

    // passing arguments to server
    if (strcmp("spawn", argv[2]) == 0) {
        dprintf(sockfd, "%d %d ", SPAWN, argc - 3);
        for (int i = 3; i < argc; ++i) {
            dprintf(sockfd, "%s ", argv[i]);
        }
    } else {
        fprintf(stderr, "invalid args were provided\n");
        return 1;
    }

    // create thread to read from server and print to stdout
    // stdin  -> socket : main
    // socket -> stdout : pthread

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, *thread_io, (void *)&sockfd);

    while (1) {
        char buf[BUF_SZ];
        ssize_t red = read(STDIN_FILENO, buf, BUF_SZ);
        if (red == 0) {
            shutdown(sockfd, SHUT_WR);
            break;
        }
        write(sockfd, buf, red);
    }
    pthread_join(thread_id, NULL);
    close(sockfd);

    return 0;
}
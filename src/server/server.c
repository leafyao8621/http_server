#include "server.h"
#include "../util/errcode/errcode.h"

#include <stdio.h>
#include <string.h>
#include <threads.h>

int HTTPServer_initialize(HTTPServer *server, uint16_t port) {
    if (!server) {
        return ERR_NULL_PTR;
    }
    server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sockfd < 0) {
        return ERR_SOCKET_OPEN;
    }
    if (
        setsockopt(
            server->sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &(int){1},
            sizeof(int)
        ) < 0) {
        return ERR_SOCKET_SET_OPT;
    }
    memset(&server->serv_addr, 0, sizeof(server->serv_addr));
    server->serv_addr.sin_family = AF_INET;
    server->serv_addr.sin_addr.s_addr = INADDR_ANY;
    server->serv_addr.sin_port = htons(port);
    if (
        bind(
            server->sockfd,
            (const struct sockaddr*)&server->serv_addr,
            sizeof(server->serv_addr)
        ) < 0) {
        return ERR_SOCKET_BIND;
    }
    server->clilen = sizeof(server->cli_addr);
    return ERR_OK;
}

int HTTPServer_finalize(HTTPServer *server) {
    if (!server) {
        return ERR_NULL_PTR;
    }
    close(server->sockfd);
    return ERR_OK;
}

struct ConnArgs {
    int sockfd;
};

int connection_handler(void *args) {
    char buf[1001];
    struct ConnArgs argv = *(struct ConnArgs*)args;
    recv(argv.sockfd, buf, 1000, 0);
    puts(buf);
    const char *msg =
        "HTTP/1.1 200 OK\n\nTEST MESSAGE";
    send(argv.sockfd, msg, 29, 0);
    shutdown(argv.sockfd, SHUT_RDWR);
    close(argv.sockfd);
    return 0;
}

int HTTPServer_start(HTTPServer *server) {
    if (listen(server->sockfd, 5)) {
        return ERR_SOCKET_LISTEN;
    }
    for (;;) {
        int client =
            accept(
                server->sockfd,
                (struct sockaddr*)&server->cli_addr,
                &server->clilen
            );
        thrd_t thread;
        struct ConnArgs args;
        args.sockfd = client;
        int ret = thrd_create(&thread, connection_handler, &args);
        if (ret) {
            return ERR_THREAD_CREATE;
        }
        ret = thrd_detach(thread);
        if (ret) {
            return ERR_THREAD_DETACH;
        }
    }
    return ERR_OK;
}

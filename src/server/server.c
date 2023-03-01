#include <stdio.h>
#include <string.h>
#include <threads.h>

#include <containers/darray.h>
#include <http_util/http_header.h>
#include <http_util/url.h>
#include <http_util/errcode.h>

#include "server.h"
#include "../util/errcode/errcode.h"

int HTTPServer_initialize(HTTPServer *server, uint16_t port) {
    if (!server) {
        return HTTP_SERVER_ERR_NULL_PTR;
    }
    server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sockfd < 0) {
        return HTTP_SERVER_ERR_SOCKET_OPEN;
    }
    if (
        setsockopt(
            server->sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &(int){1},
            sizeof(int)
        ) < 0) {
        return HTTP_SERVER_ERR_SOCKET_SET_OPT;
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
        return HTTP_SERVER_ERR_SOCKET_BIND;
    }
    server->clilen = sizeof(server->cli_addr);
    return HTTP_SERVER_ERR_OK;
}

int HTTPServer_finalize(HTTPServer *server) {
    if (!server) {
        return HTTP_SERVER_ERR_NULL_PTR;
    }
    close(server->sockfd);
    return HTTP_SERVER_ERR_OK;
}

struct ConnArgs {
    int sockfd;
};

int connection_handler(void *args) {
    char raw_buf[1001];
    String buf;
    int ret = DArrayChar_initialize(&buf, 1001);
    if (ret) {
        return 0;
    }
    struct ConnArgs argv = *(struct ConnArgs*)args;
    ssize_t sz = 0;
    for (; (sz = recv(argv.sockfd, raw_buf, 1000, 0)) == 1000;) {
        ret = DArrayChar_push_back_batch(&buf, raw_buf, 1000);
        if (ret) {
            return 0;
        }
    }
    ret = DArrayChar_push_back_batch(&buf, raw_buf, sz);
    if (ret) {
        return 0;
    }
    char chr = 0;
    ret = DArrayChar_push_back(&buf, &chr);
    if (ret) {
        return 0;
    }
    puts(buf.data);
    char *iter = buf.data;
    for (; *iter != 0 && *iter != '/'; ++iter);
    URL url;
    ret = URL_initialize(&url);
    if (ret) {
        DArrayChar_finalize(&buf);
        return 0;
    }
    String out_buf;
    ret = DArrayChar_initialize(&out_buf, 1000);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        return 0;
    }
    ret = URL_parse(&url, &iter);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        DArrayChar_finalize(&out_buf);
        return 0;
    }
    ret = URL_serialize(&url, &out_buf);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        DArrayChar_finalize(&out_buf);
        return 0;
    }
    DArrayChar_pop_back(&out_buf);
    for (; *iter != '\n'; ++iter);
    ++iter;
    HTTPHeader header;
    ret = HTTPHeader_initialize(&header);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        DArrayChar_finalize(&out_buf);
        return 0;
    }
    ret = HTTPHeader_parse(&header, &iter);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        DArrayChar_finalize(&out_buf);
        return 0;
    }
    ret = HTTPHeader_serialize(&header, &out_buf);
    if (ret) {
        DArrayChar_finalize(&buf);
        URL_finalize(&url);
        HTTPHeader_finalize(&header);
        DArrayChar_finalize(&out_buf);
        return 0;
    }
    puts(out_buf.data);
    DArrayChar_finalize(&out_buf);
    URL_finalize(&url);
    HTTPHeader_finalize(&header);
    DArrayChar_finalize(&buf);
    const char *msg =
        "HTTP/1.1 200 OK\n\nTEST MESSAGE";
    send(argv.sockfd, msg, 29, 0);
    shutdown(argv.sockfd, SHUT_RDWR);
    close(argv.sockfd);
    return 0;
}

int HTTPServer_start(HTTPServer *server) {
    if (listen(server->sockfd, 5)) {
        return HTTP_SERVER_ERR_SOCKET_LISTEN;
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
            return HTTP_SERVER_ERR_THREAD_CREATE;
        }
        ret = thrd_detach(thread);
        if (ret) {
            return HTTP_SERVER_ERR_THREAD_DETACH;
        }
    }
    return HTTP_SERVER_ERR_OK;
}

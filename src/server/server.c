#include <stdio.h>
#include <string.h>
#include <threads.h>

#include <containers/darray.h>
#include <http_util/errcode.h>

#include "server.h"
#include "router.h"
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
    int ret = Router_initialize(&server->router);
    if (ret) {
        return HTTP_SERVER_ERR_ROUTER_INITIALIZE;
    }
    return HTTP_SERVER_ERR_OK;
}

int HTTPServer_finalize(HTTPServer *server) {
    if (!server) {
        return HTTP_SERVER_ERR_NULL_PTR;
    }
    close(server->sockfd);
    Router_finalize(&server->router);
    return HTTP_SERVER_ERR_OK;
}

struct ConnArgs {
    int sockfd;
    HTTPServer *server;
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
    HTTPRequest request;
    ret = HTTPRequest_initialize(&request);
    if (ret) {
        DArrayChar_finalize(&buf);
        return 0;
    }
    ret = HTTPRequest_parse(&request, buf.data);
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        return 0;
    }
    HTTPResponse response;
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        HTTPResponse_finalize(&response);
        return 0;
    }
    String res_buf;
    ret = DArrayChar_initialize(&res_buf, 1000);
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        HTTPResponse_finalize(&response);
        return 0;
    }
    URLMethod url_method;
    url_method.url = request.url;
    url_method.method = request.method;
    bool found;
    ret =
        HashMapURLMethodHTTPRequestHandler_find(
            &argv.server->router,
            &url_method,
            &found
        );
    HTTPRequestHandler *tgt;
    if (!found) {
        HashMapURLMethodHTTPRequestHandlerNode *iter =
            argv.server->router.data;
        for (size_t i = 0; i < argv.server->router.capacity; ++i, ++iter) {
            if (iter->in_use) {
                if (argv.server->router.eq(&iter->key, &url_method)) {
                    tgt = &iter->value;
                    found = true;
                    break;
                }
            }
        }
    } else {
        ret =
            HashMapURLMethodHTTPRequestHandler_fetch(
                &argv.server->router,
                &url_method,
                &tgt
            );
    }
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        HTTPResponse_finalize(&response);
        DArrayChar_finalize(&res_buf);
        return 0;
    }
    if (found) {
        ret = (*tgt)(&request, &response);
    } else {
        ret =
            HTTPResponse_initialize(
                &response,
                HTTP_RESPONSE_404,
                BODY_TYPE_TEXT
            );
        ret =
            DArrayChar_push_back_batch(
                &response.body.text,
                "Invalid Request",
                16
            );
    }
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        HTTPResponse_finalize(&response);
        DArrayChar_finalize(&res_buf);
        return 0;
    }
    ret = HTTPResponse_serialize(&response, &res_buf);
    if (ret) {
        DArrayChar_finalize(&buf);
        HTTPRequest_finalize(&request);
        HTTPResponse_finalize(&response);
        DArrayChar_finalize(&res_buf);
        return 0;
    }
    send(argv.sockfd, res_buf.data, res_buf.size - 1, 0);
    DArrayChar_finalize(&buf);
    HTTPRequest_finalize(&request);
    HTTPResponse_finalize(&response);
    DArrayChar_finalize(&res_buf);
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
        args.server = server;
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

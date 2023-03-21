#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <http_util/http_request_response.h>

typedef struct URLMethod {
    URL url;
    HTTPMethod method;
} URLMethod;

typedef int (*HTTPRequestHandler)(HTTPRequest*, HTTPResponse*);

DEF_HASHMAP(URLMethod, HTTPRequestHandler)

typedef HashMapURLMethodHTTPRequestHandler Router;

typedef struct HTTPServer {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    Router router;
} HTTPServer;

int HTTPServer_initialize(HTTPServer *server, uint16_t port);
int HTTPServer_start(HTTPServer *server);
int HTTPServer_finalize(HTTPServer *server);

int HTTPServer_set_route(
    HTTPServer *server,
    char *route,
    HTTPMethod method,
    HTTPRequestHandler handler);

#endif

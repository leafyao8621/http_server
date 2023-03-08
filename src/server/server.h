#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <http_util/http_header.h>
#include <http_util/url.h>
#include <json/document.h>

typedef enum HTTPMethod {
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH
} HTTPMethod;

typedef struct HTTPRequest {
    HTTPMethod method;
    URL url;
    URLParams params;
    HTTPHeader header;
    union {
        String text;
        URLParams url_encoded;
        JSONDocument json;
    } body;
} HTTPRequest;

typedef int (*HTTPRequestHandler)(HTTPRequest*);

typedef struct HTTPServer {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
} HTTPServer;

int HTTPServer_initialize(HTTPServer *server, uint16_t port);
int HTTPServer_start(HTTPServer *server);
int HTTPServer_finalize(HTTPServer *server);

#endif

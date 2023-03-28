#include <stdio.h>

#include <containers/eq.h>
#include <containers/hash.h>

#include "router.h"
#include "../util/errcode/errcode.h"

DEF_HASHMAP_FUNCTIONS(URLMethod, HTTPRequestHandler)

bool eq_url_method(URLMethod *a, URLMethod *b) {
    if (a->method != b->method) {
        return false;
    }
    if (containers_eq_dstr(&a->url.text, &b->url.text)) {
        return true;
    }
    if (a->url.breakdown.size != b->url.breakdown.size) {
        return false;
    }
    String *iter_a = a->url.breakdown.data;
    String *iter_b = b->url.breakdown.data;
    for (size_t i = 0; i < a->url.breakdown.size; ++i, ++iter_a, ++iter_b) {
        if (iter_a->size == 2 && iter_a->data[0] == '*') {
            continue;
        }
        if (!containers_eq_dstr(iter_a, iter_b)) {
            return false;
        }
    }
    return true;
}

size_t hash_url_method(URLMethod *a) {
    return containers_hash_dstr(&a->url.text) ^ a->method;
}

int Router_initialize(Router *router) {
    int ret =
        HashMapURLMethodHTTPRequestHandler_initialize(
            router,
            10,
            eq_url_method,
            hash_url_method
        );
    if (ret) {
        return 1;
    }
    return 0;
}

int Router_finalize(Router *router) {
    HashMapURLMethodHTTPRequestHandlerNode *iter = router->data;
    for (size_t i = 0; i < router->capacity; ++i, ++iter) {
        if (iter->in_use) {
            URL_finalize(&iter->key.url);
        }
    }
    HashMapURLMethodHTTPRequestHandler_finalize(router);
    return 0;
}

int HTTPServer_set_route(
    HTTPServer *server,
    char *route,
    HTTPMethod method,
    HTTPRequestHandler handler) {
    URLMethod url_method;
    int ret = URL_initialize(&url_method.url);
    if (ret) {
        return HTTP_SERVER_ERR_SET_ROUTE;
    }
    ret = URL_parse(&url_method.url, &route);
    if (ret) {
        return HTTP_SERVER_ERR_SET_ROUTE;
    }
    url_method.method = method;
    HTTPRequestHandler *tgt;
    ret =
        HashMapURLMethodHTTPRequestHandler_fetch(
            &server->router,
            &url_method,
            &tgt
        );
    if (ret) {
        return HTTP_SERVER_ERR_SET_ROUTE;
    }
    *tgt = handler;
    return HTTP_SERVER_ERR_OK;
}

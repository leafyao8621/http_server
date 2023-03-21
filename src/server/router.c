#include <containers/eq.h>
#include <containers/hash.h>

#include "router.h"

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

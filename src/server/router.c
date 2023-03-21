#include "router.h"

DEF_HASHMAP_FUNCTIONS(URLMethod, HTTPRequestHandler)

int Router_initialize(Router *router) {
    int ret =
        HashMapURLMethodHTTPRequestHandler_initialize(
            router,
            10,
            0,
            0
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

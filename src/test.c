#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <http_server/errcode.h>
#include <http_server/server.h>
#include <containers/eq.h>
#include <containers/hash.h>

HTTPServer server;

int add_post_handler(HTTPRequest *request, HTTPResponse *response) {
    if (request->body_type != BODY_TYPE_JSON) {
        int ret =
            HTTPResponse_initialize(
                response,
                HTTP_RESPONSE_400,
                BODY_TYPE_TEXT
            );
        if (ret) {
            return 1;
        }
        ret = DArrayChar_push_back_batch(&response->body.text, "Invalid!", 9);
        if (ret) {
            return 1;
        }
        return 0;
    }
    int ret =
        HTTPResponse_initialize(
            response,
            HTTP_RESPONSE_200,
            BODY_TYPE_JSON
        );
    if (ret) {
        return 1;
    }
    response->body.json.root->is_null = false;
    response->body.json.root->type = OBJECT;
    ret =
        HashMapStringJSONNodePtr_initialize(
            &response->body.json.root->data.object,
            2,
            containers_eq_dstr,
            containers_hash_dstr
        );
    if (ret) {
        return 1;
    }
    String key;
    ret = DArrayChar_initialize(&key, 10);
    if (ret) {
        return 1;
    }
    ret = DArrayChar_push_back_batch(&key, "a", 2);
    if (ret) {
        return 1;
    }
    JSONNodePtr *tgt;
    bool success = true;
    bool found;
    double a, b;
    ret =
        HashMapStringJSONNodePtr_find(
            &request->body.json.root->data.object,
            &key,
            &found
        );
    if (!found) {
        success = false;
    }
    if (success) {
        ret =
            HashMapStringJSONNodePtr_fetch(
                &request->body.json.root->data.object,
                &key,
                &tgt
            );
        if ((*tgt)->is_null || (*tgt)->type != NUMBER) {
            success = false;
        }
        if (success) {
            a = (*tgt)->data.number;
            DArrayChar_clear(&key);
            ret = DArrayChar_push_back_batch(&key, "b", 2);
            ret =
                HashMapStringJSONNodePtr_find(
                    &request->body.json.root->data.object,
                    &key,
                    &found
                );
            if (!found) {
                success = false;
            }
            if (success) {
                ret =
                    HashMapStringJSONNodePtr_fetch(
                        &request->body.json.root->data.object,
                        &key,
                        &tgt
                    );
                if ((*tgt)->is_null || (*tgt)->type != NUMBER) {
                    success = false;
                }
                if (success) {
                    b = (*tgt)->data.number;
                }
            }
        }
    }
    DArrayChar_finalize(&key);
    ret = DArrayChar_initialize(&key, 10);
    if (ret) {
        return 1;
    }
    ret = DArrayChar_push_back_batch(&key, "success", 8);
    if (ret) {
        return 1;
    }
    ret =
        HashMapStringJSONNodePtr_fetch(
            &response->body.json.root->data.object,
            &key,
            &tgt
        );
    if (ret) {
        DArrayChar_finalize(&key);
        return 1;
    }
    ret = JSONNodePtr_initialize(tgt);
    if (ret) {
        return 1;
    }
    (*tgt)->is_null = false;
    (*tgt)->type = BOOLEAN;
    (*tgt)->data.boolean = success;
    ret = DArrayChar_initialize(&key, 10);
    if (ret) {
        return 1;
    }
    ret = DArrayChar_push_back_batch(&key, "result", 7);
    if (ret) {
        return 1;
    }
    ret =
        HashMapStringJSONNodePtr_fetch(
            &response->body.json.root->data.object,
            &key,
            &tgt
        );
    if (ret) {
        DArrayChar_finalize(&key);
        return 1;
    }
    ret = JSONNodePtr_initialize(tgt);
    if (ret) {
        return 1;
    }
    (*tgt)->is_null = false;
    (*tgt)->type = NUMBER;
    (*tgt)->data.number = a + b;
    return 0;
}

bool is_anagram(String *str) {
    char *iter_front, *iter_back;
    iter_front = str->data;
    iter_back = str->data + str->size - 2;
    for (
        size_t i = 0;
        i < (str->size - 1) >> 1;
        ++i, ++iter_front, --iter_back) {
        if (*iter_front != *iter_back) {
            return false;
        }
    }
    return true;
}

int anagram_get_handler(HTTPRequest *request, HTTPResponse *response) {
    bool res = is_anagram(&request->url.breakdown.data[1]);
    int ret =
        HTTPResponse_initialize(
            response,
            HTTP_RESPONSE_200,
            BODY_TYPE_JSON
        );
    if (ret) {
        return 1;
    }
    response->body.json.root->is_null = false;
    response->body.json.root->type = OBJECT;
    ret =
        HashMapStringJSONNodePtr_initialize(
            &response->body.json.root->data.object,
            2,
            containers_eq_dstr,
            containers_hash_dstr
        );
    if (ret) {
        return 1;
    }
    String key;
    ret = DArrayChar_initialize(&key, 10);
    if (ret) {
        return 1;
    }
    JSONNodePtr *tgt;
    ret = DArrayChar_push_back_batch(&key, "result", 7);
    if (ret) {
        return 1;
    }
    ret =
        HashMapStringJSONNodePtr_fetch(
            &response->body.json.root->data.object,
            &key,
            &tgt
        );
    if (ret) {
        DArrayChar_finalize(&key);
        return 1;
    }
    ret = JSONNodePtr_initialize(tgt);
    if (ret) {
        return 1;
    }
    (*tgt)->is_null = false;
    (*tgt)->type = BOOLEAN;
    (*tgt)->data.boolean = res;
    return 0;
}

void sig_handler(int sig) {
    printf("Received signal %d\n", sig);
    int ret = HTTPServer_finalize(&server);
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    exit(0);
}

int main(void) {
    int ret = HTTPServer_initialize(&server, 8000);
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    ret =
        HTTPServer_set_route(
            &server,
            "/add ",
            HTTP_METHOD_POST,
            add_post_handler
        );
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    ret =
        HTTPServer_set_route(
            &server,
            "/anagram/* ",
            HTTP_METHOD_GET,
            anagram_get_handler
        );
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGTERM, sig_handler);
    HTTPServer_start(&server);
    return 0;
}

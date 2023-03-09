#include "server.h"

#include <stdio.h>

const char *http_method_lookup[9] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "PATCH"
};

int HTTPMethod_parse(HTTPMethod *method, char **iter) {
    String buf;
    int ret = DArrayChar_initialize(&buf, 10);
    if (ret) {
        return 1;
    }
    for (; **iter && **iter != ' '; ++(*iter)) {
        ret = DArrayChar_push_back(&buf, *iter);
        if (ret) {
            DArrayChar_finalize(&buf);
            return 1;
        }
    }
    char chr = 0;
    ret = DArrayChar_push_back(&buf, &chr);
    if (ret) {
        DArrayChar_finalize(&buf);
        return 1;
    }
    const char **lookup = http_method_lookup;
    for (*method = 0; *method < 9; ++(*method), ++lookup) {
        if (!strcmp(*lookup, buf.data)) {
            DArrayChar_finalize(&buf);
            return 0;
        }
    }
    DArrayChar_finalize(&buf);
    return 1;
}

int HTTPRequest_initialize(HTTPRequest *request) {
    int ret = URL_initialize(&request->url);
    if (ret) {
        return 1;
    }
    ret = URLParams_initialize(&request->params);
    if (ret) {
        return 1;
    }
    ret = HTTPHeader_initialize(&request->header);
    if (ret) {
        return 1;
    }
    request->body_type = BODY_TYPE_NONE;
    return 0;
}

int HTTPRequest_parse(HTTPRequest *request, char *data) {
    char *iter = data;
    int ret = HTTPMethod_parse(&request->method, &iter);
    if (ret) {
        return 1;
    }
    for (; *iter && *iter != '/'; ++iter);
    ret = URL_parse(&request->url, &iter);
    if (ret) {
        return 1;
    }
    if (*iter == '?') {
        ret = URLParams_parse(&request->params, &iter);
    }
    for (; *iter != '\n'; ++iter);
    ++iter;
    ret = HTTPHeader_parse(&request->header, &iter);
    if (ret) {
        return 1;
    }
    ++iter;
    String buf;
    ret = DArrayChar_initialize(&buf, 20);
    if (ret) {
        return 1;
    }
    ret = DArrayChar_push_back_batch(&buf, "Content-Type", 13);
    if (ret) {
        DArrayChar_finalize(&buf);
        return 1;
    }
    puts(buf.data);
    bool found = false;
    ret = HashMapStringString_find(&request->header, &request->header.data[6].key, &found);
    if (ret) {
        DArrayChar_finalize(&buf);
        return 1;
    }
    printf("%d xxsdsf\n", found);
    if (found) {
        String *temp;
        ret = HashMapStringString_fetch(&request->header, &request->header.data[6].key, &temp);
        if (ret) {
            DArrayChar_finalize(&buf);
            return 1;
        }
        puts(temp->data);
        if (!strcmp(temp->data, "application/json")) {
            ret = JSONDocument_parse(&request->body.json, iter);
            if (ret) {
                DArrayChar_finalize(&buf);
                JSONDocument_finalize(&request->body.json);
                return 1;
            }
            request->body_type = BODY_TYPE_JSON;
        } else if (!strcmp(temp->data, "application/x-www-form-urlencoded")) {
            ret = URLParams_initialize(&request->body.url_encoded);
            if (ret) {
                DArrayChar_finalize(&buf);
                return 1;
            }
            ret = URLParams_parse(&request->body.url_encoded, &iter);
            if (ret) {
                DArrayChar_finalize(&buf);
                URLParams_finalize(&request->body.url_encoded);
                return 1;
            }
            request->body_type = BODY_TYPE_URL_ENCODED;
        } else {
            size_t len = strlen(iter) + 1;
            ret = DArrayChar_initialize(&request->body.text, len + 1);
            if (ret) {
                DArrayChar_finalize(&buf);
                return 1;
            }
            ret =
                DArrayChar_push_back_batch(&request->body.text, iter, len);
            if (ret) {
                DArrayChar_finalize(&buf);
                DArrayChar_finalize(&request->body.text);
                return 1;
            }
            request->body_type = BODY_TYPE_TEXT;
        }
    } else {
        size_t len = strlen(iter) + 1;
        ret = DArrayChar_initialize(&request->body.text, len + 1);
        if (ret) {
            DArrayChar_finalize(&buf);
            return 1;
        }
        ret =
            DArrayChar_push_back_batch(&request->body.text, iter, len);
        if (ret) {
            DArrayChar_finalize(&buf);
            DArrayChar_finalize(&request->body.text);
            return 1;
        }
        request->body_type = BODY_TYPE_TEXT;
    }
    DArrayChar_finalize(&buf);
    return 0;
}

int HTTPRequest_finalize(HTTPRequest *request) {
    URL_finalize(&request->url);
    URLParams_finalize(&request->params);
    HTTPHeader_finalize(&request->header);
    switch (request->body_type) {
    case BODY_TYPE_NONE:
        break;
    case BODY_TYPE_TEXT:
        DArrayChar_finalize(&request->body.text);
        break;
    case BODY_TYPE_URL_ENCODED:
        URLParams_finalize(&request->body.url_encoded);
        break;
    case BODY_TYPE_JSON:
        JSONDocument_finalize(&request->body.json);
        break;
    }
    return 0;
}

int HTTPRequest_serialize(HTTPRequest *request, String *buf) {
    char *method = (char*)http_method_lookup[request->method];
    if (DArrayChar_push_back_batch(buf, method, strlen(method))) {
        return 1;
    }
    char chr = '\n';
    if (DArrayChar_push_back(buf, &chr)) {
        return 1;
    }
    if (URL_serialize(&request->url, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    if (URLParams_serialize(&request->params, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    if (DArrayChar_push_back(buf, &chr)) {
        return 1;
    }
    if (HTTPHeader_serialize(&request->params, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    if (HTTPHeader_serialize(&request->header, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    switch (request->body_type) {
    case BODY_TYPE_NONE:
        break;
    case BODY_TYPE_TEXT:
        if (
            DArrayChar_push_back_batch(
                buf,
                request->body.text.data,
                request->body.text.size
            )) {
            return 1;
        }
        break;
    case BODY_TYPE_URL_ENCODED:
        if (URLParams_serialize(&request->body.url_encoded, buf)) {
            return 1;
        }
        DArrayChar_pop_back(buf);
        if (DArrayChar_push_back(buf, &chr)) {
            return 1;
        }
        if (HTTPHeader_serialize(&request->params, buf)) {
            return 1;
        }
        break;
    case BODY_TYPE_JSON:
        if(JSONDocument_serialize(&request->body.json, buf, false)) {
            return 1;
        }
        break;
    }
    return 0;
}

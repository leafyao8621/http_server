#include "server.h"

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
    return 0;
}

int HTTPRequest_parse(HTTPRequest *request, char *data) {
    char *iter = data;
    for (; *iter != 0 && *iter != '/'; ++iter);
    int ret = URL_parse(&request->url, &iter);
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
    return 0;
}

int HTTPRequest_finalize(HTTPRequest *request) {
    URL_finalize(&request->url);
    URLParams_finalize(&request->params);
    HTTPHeader_finalize(&request->header);
    return 0;
}

int HTTPRequest_serialize(HTTPRequest *request, String *buf) {
    if (URL_serialize(&request->url, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    if (URLParams_serialize(&request->params, buf)) {
        return 1;
    }
    DArrayChar_pop_back(buf);
    char chr = '\n';
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
    return 0;
}

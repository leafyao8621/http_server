#include "server.h"

const char *http_response_code_lookup[63] = {
    "100 Continue",
    "101 Switching Protocols",
    "102 Processing",
    "103 Early Hints",
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authoritative Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    "207 Multi-Status",
    "208 Already Reported",
    "226 IM Used",
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Found",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "306 unused",
    "307 Temporary Redirect",
    "308 Permanent Redirect",
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Timeout",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Payload Too Large",
    "414 URI Too Long",
    "415 Unsupported Media Type",
    "416 Range Not Satisfiable",
    "417 Expectation Failed",
    "418 I'm a teapot",
    "421 Misdirected Request",
    "422 Unprocessable Content",
    "423 Locked",
    "424 Failed Dependency",
    "425 Too Early",
    "426 Upgrade Required",
    "428 Precondition Required",
    "429 Too Many Requests",
    "431 Request Header Fields Too Large",
    "451 Unavailable For Legal Reasons",
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Timeout",
    "505 HTTP Version Not Supported",
    "506 Variant Also Negotiates",
    "507 Insufficient Storage",
    "508 Loop Detected",
    "510 Not Extended",
    "511 Network Authentication Required"
};

int HTTPResponse_initialize(HTTPResponse *response, HTTPBodyType body_type) {
    if (!response) {
        return 1;
    }
    int ret = HTTPHeader_initialize(&response->header);
    if (ret) {
        return 1;
    }
    response->body_type = body_type;
    switch (body_type) {
    case BODY_TYPE_NONE:
        break;
    case BODY_TYPE_TEXT:
        ret = DArrayChar_initialize(&response->body, 100);
        break;
    case BODY_TYPE_URL_ENCODED:
        ret = URLParams_initialize(&response->body.url_encoded);
        break;
    case BODY_TYPE_JSON:
        ret = JSONNodePtr_initialize(&response->body.json);
        break;
    }
    if (ret) {
        return 1;
    }
    return 0;
}

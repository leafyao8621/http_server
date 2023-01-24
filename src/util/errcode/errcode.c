#include "errcode.h"

const char *errcode_lookup[8] = {
    "ERR_OK",
    "ERR_SOCKET_OPEN",
    "ERR_SOCKET_BIND",
    "ERR_NULL_PTR",
    "ERR_SOCKET_LISTEN",
    "ERR_THREAD_CREATE",
    "ERR_THREAD_DETACH",
    "ERR_SOCKET_SET_OPT",
};

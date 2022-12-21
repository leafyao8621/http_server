#ifndef UTIL_ERRCODE_ERRCODE_H_
#define UTIL_ERRCODE_ERRCODE_H_

#define ERR_OK 0
#define ERR_SOCKET_OPEN 1
#define ERR_SOCKET_BIND 2
#define ERR_NULL_PTR 3
#define ERR_SOCKET_LISTEN 4
#define ERR_THREAD_CREATE 5
#define ERR_THREAD_DETACH 6
#define ERR_SOCKET_SET_OPT 7

extern const char *errcode_lookup[8];

#endif

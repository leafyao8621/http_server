#ifndef UTIL_DBUF_DBUF_H_
#define UTIL_DBUF_DBUF_H_

#include <stdint.h>

typedef struct DBuf {
    uint64_t size, capacity;
    char *data;
} DBuf;

#endif

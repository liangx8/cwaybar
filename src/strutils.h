#ifndef STRUTILS_H_KKDIEIE3
#define STRUTILS_H_KKDIEIE3

#include <stddef.h>

struct HOLDER_REPLACE{
    const char *holder;
    const char *replace;
};

int str_transfer(char *dst,
                 size_t *dst_size,
                 const char *src,
                 const struct HOLDER_REPLACE hr[]);

#endif
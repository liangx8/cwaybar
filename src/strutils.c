#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "strutils.h"

static const struct HOLDER_REPLACE *find_holder(const char *src,
                                                const struct HOLDER_REPLACE hr[],
                                                size_t *holder_length)
{
    for (size_t i = 0; hr[i].holder; i++) {
        size_t length = strlen(hr[i].holder);
        if (strncmp(src, hr[i].holder, length) == 0) {
            *holder_length = length;
            return &hr[i];
        }
    }
    return NULL;
}

int str_replace_home(char *dst,size_t *dst_size,const char *cstr)
{
    static const char placeholder[] = "{home}";
    const size_t placeholder_length = sizeof(placeholder) - 1;
    const size_t alignment = 8;
    const char *home;
    const char *cursor;
    const char *match;
    size_t home_length;
    size_t output_length = 0;
    size_t aligned_size;
    size_t written = 0;

    if (!dst || !dst_size || !cstr || ((uintptr_t)dst & (alignment - 1)) != 0) {
        return -1;
    }

    cursor = cstr;
    match = strstr(cursor, placeholder);
    if (match) {
        home = getenv("HOME");
        if (!home) {
            return -1;
        }
        home_length = strlen(home);
    } else {
        home = NULL;
        home_length = 0;
    }

    while (match) {
        size_t segment_length = (size_t)(match - cursor);
        if (segment_length > SIZE_MAX - output_length ||
            home_length > SIZE_MAX - output_length - segment_length) {
            return -1;
        }
        output_length += segment_length + home_length;
        cursor = match + placeholder_length;
        match = strstr(cursor, placeholder);
    }

    size_t tail_length = strlen(cursor);
    if (tail_length > SIZE_MAX - output_length) {
        return -1;
    }
    output_length += tail_length;

    if (output_length == SIZE_MAX || output_length + 1 > SIZE_MAX - (alignment - 1)) {
        return -1;
    }
    aligned_size = (output_length + 1 + alignment - 1) & ~(alignment - 1);
    if (aligned_size > *dst_size) {
        return -1;
    }

    cursor = cstr;
    match = strstr(cursor, placeholder);
    while (match) {
        size_t segment_length = (size_t)(match - cursor);
        memcpy(dst + written, cursor, segment_length);
        written += segment_length;
        memcpy(dst + written, home, home_length);
        written += home_length;
        cursor = match + placeholder_length;
        match = strstr(cursor, placeholder);
    }
    memcpy(dst + written, cursor, tail_length);
    written += tail_length;
    dst[written] = '\0';
    memset(dst + written + 1, 0, aligned_size - written - 1);
    *dst_size -= aligned_size;
    return 0;
}

int str_transfer(char *dst,
                 size_t *dst_size,
                 const char *src,
                 const struct HOLDER_REPLACE hr[])
{
    const size_t alignment = 8;
    const char *cursor;
    size_t output_length = 0;
    size_t aligned_size;
    size_t written = 0;

    if (!dst || !dst_size || !src || !hr ||
        ((uintptr_t)dst & (alignment - 1)) != 0) {
        return -1;
    }

    for (size_t i = 0; hr[i].holder; i++) {
        if (hr[i].holder[0] == '\0' || !hr[i].replace) {
            return -1;
        }
    }

    cursor = src;
    while (*cursor) {
        size_t holder_length = 0;
        const struct HOLDER_REPLACE *holder =
            find_holder(cursor, hr, &holder_length);
        size_t added_length;

        if (holder) {
            added_length = strlen(holder->replace);
            cursor += holder_length;
        } else {
            added_length = 1;
            cursor++;
        }
        if (added_length > SIZE_MAX - output_length) {
            return -1;
        }
        output_length += added_length;
    }

    if (output_length == SIZE_MAX ||
        output_length + 1 > SIZE_MAX - (alignment - 1)) {
        return -1;
    }
    aligned_size = (output_length + 1 + alignment - 1) & ~(alignment - 1);
    if (aligned_size > *dst_size) {
        return -1;
    }

    cursor = src;
    while (*cursor) {
        size_t holder_length = 0;
        const struct HOLDER_REPLACE *holder =
            find_holder(cursor, hr, &holder_length);

        if (holder) {
            size_t replacement_length = strlen(holder->replace);
            if (replacement_length > 0) {
                memcpy(dst + written, holder->replace, replacement_length);
                written += replacement_length;
            }
            cursor += holder_length;
        } else {
            dst[written++] = *cursor++;
        }
    }

    dst[written] = '\0';
    memset(dst + written + 1, 0, aligned_size - written - 1);
    *dst_size -= aligned_size;
    return 0;
}
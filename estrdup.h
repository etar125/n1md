/*
 * Copyright (c) 2026 etar125 Admanse
 *
 * Copying and distribution of this file, with or without modification, are permitted in any medium without royalty, provided the copyright notice and this notice are preserved. This file is offered as-is, without any warranty.
 */

#ifndef ESTRDUP_H
#define ESTRDUP_H

#include <stdlib.h>
#include <string.h>

#ifdef ESTRDUP

static char* estrdup(const char *s) {
    if (!s) { return NULL; }
    size_t len = strlen(s);
    char *ret = malloc(len + 1);
    if (!ret) { return NULL; }
    memcpy(ret, s, len);
    ret[len] = '\0';
    return ret;
}

#endif

#ifdef ESTRDUPL

static char* estrdupl(const char *s, size_t *outlen) {
    if (!s) { return NULL; }
    size_t len = strlen(s);
    char *ret = malloc(len + 1);
    if (!ret) { return NULL; }
    memcpy(ret, s, len);
    ret[len] = '\0';
    if (outlen) { *outlen = len; }
    return ret;
}

#endif

#ifdef ESTRNDUP

static char* estrndup(const char *s, size_t n) {
    if (!s) { return NULL; }
    
    size_t len = 0;
    for (; len < n && s[len] != '\0'; len++);

    char *ret = malloc(len + 1);
    if (!ret) { return NULL; }
    memcpy(ret, s, len);
    ret[len] = '\0';
    return ret;
}

#endif

#ifdef ESTRNDUPL

static char* estrndupl(const char *s, size_t n, size_t *outlen) {
    if (!s) { return NULL; }
    
    size_t len = 0;
    for (; len < n && s[len] != '\0'; len++);

    char *ret = malloc(len + 1);
    if (!ret) { return NULL; }
    memcpy(ret, s, len);
    ret[len] = '\0';
    if (outlen) { *outlen = len; }
    return ret;
}

#endif

#endif

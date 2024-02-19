/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "zmalloc.h"

#ifdef _WIN32
#define PATH_SEPARATOR   '\\'
#else
#define PATH_SEPARATOR   '/'
#endif

static char *path_normalize(const char *path) {
    if (!path) return NULL;

    char *copy = zstrdup(path);
    if (NULL == copy) return NULL;
    char *ptr = copy;

    for (int i = 0; copy[i]; i++) {
        *ptr++ = path[i];
        if ('/' == path[i]) {
            i++;
            while ('/' == path[i]) i++;
            i--;
        }
    }
    *ptr = '\0';

    return copy;
}

int kx_mkdirp(const char *path, unsigned int mode) {
    char *pathname = NULL;
    char *parent = NULL;

    if (NULL == path) return -1;

    pathname = path_normalize(path);
    if (NULL == pathname) goto fail;

    parent = zstrdup(pathname);
    if (NULL == parent) goto fail;

    char *p = parent + strlen(parent);
    while (PATH_SEPARATOR != *p && p != parent) {
        p--;
    }
    *p = '\0';

    // make parent dir
    if (p != parent && 0 != kx_mkdirp(parent, mode))
        goto fail;
    zfree(parent);

    // make this one if parent has been made
    #ifdef _WIN32
        int rc = mkdir(pathname);
    #else
        int rc = mkdir(pathname, mode);
    #endif

    zfree(pathname);

    return 0 == rc || EEXIST == errno
        ? 0
        : -1;

fail:
    zfree(pathname);
    zfree(parent);
    return -1;
}
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
#ifndef __KX_RKX_H__
#define __KX_RKX_H__

#include "rkxconfig.h"
#include "xxhash.h"
#include "zmalloc.h"
#include "log.h"
#include "node.h"
#include "user.h"
#include "file.h"
#include "adlist.h"
#include "net.h"
#include "aes.h"
#include "db.h"
#include "mq.h"

#define MAXMAPSIZE  (10 * 1024 * 1024)

struct kxoption {
    struct kxoption *next;
    char *key;
    char *opt_string;
    char *value;
};

struct options {
    struct kxoption *kx_options;
};
struct context {
    struct options *options;
    int argc;
    char **argv;
};

struct kxclient {
    kxnode *node;               /* client node A machine can only have one node*/
    kxuser *user;               /* User Info */
    kxsyncnet *net;
    kxdb *db;
    kxmq *mq;
    list *local_cryptfiles;     /* Local encrypted files */
    list *remote_cryptfiles;    /* Encrypt files remotely */
    pthread_t ptdnet;
    pthread_rwlock_t rwlock;
};

/** Initialize client node
 * @note Only called once when the server starts
 */
void rkx_init(struct kxclient *kx);

extern struct kxclient client;

#endif
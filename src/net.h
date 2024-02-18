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
#ifndef __KX_NET_H__
#define __KX_NET_H__

#include "rkxconfig.h"

typedef enum kxtype {
    NODE_SET = 0x01,
    NODE_GET,
    USER_REG,
    USER_LOGIN,
    USER_GET,
    FILE_CRYPT,         /* STORE crypt file infomation*/
    FILE_GETLIST,        /* Query information about all encrypted files on the current device */
    FILE_GET
} kxtype;

typedef struct kxnet {
    redisAsyncContext *context;
    struct event_base *base;
    redisOptions options;
} kxnet;

typedef struct kxsyncnet {
    redisContext *context;
} kxsyncnet;

typedef void (*callback)(redisAsyncContext *c, void *r, void *privdata);
typedef void (*synccallback)(redisReply *c);
struct action {
    kxtype type;
    char *cmdline;
    callback exec;
    synccallback syncexec;
    redisReply *reply;
};
/**
 * free kxnet object
 */
void kx_free_net(kxsyncnet *net);

/** search action by type
 * 
 * @param type action type
 * @return return const action
 */
struct action *kx_search_action(kxtype type);

/** async send message
 * @param net kxnet object
 * @param format command format eg. SET key value
 */
void kx_async_send_cmd(kxnet *net, struct action *ac, const char *fmt, ...);

/** Create redis sync link object
 * 
 * @param addr redis-server ip address eg. : 127.0.0.1
 * @param port redis-server port eg. : 6790
 * @return Returns the created object, or NULL if failed
 */
kxsyncnet *kx_sync_creat_net(const char *addr, uint32_t port);

/** sync send message
 * @param net kxsyncnet object
 * @param format command format eg. SET key value
 */
void kx_sync_send_cmd(kxsyncnet *net, struct action *ac, const char *fmt, ...);

#endif
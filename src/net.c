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
#include "log.h"
#include "net.h"
#include "zmalloc.h"

static void kx_command(redisContext *c, const char *cmd, struct action *ac);
static void kx_set_reply(redisReply *reply);

/**
 * HMSET node:f526255265340d994510f8d1652e1eb1 uuid f526255265340d994510f8d1652e1eb1 ip 127.0.0.1
 * HGETALL node:f526255265340d994510f8d1652e1eb1
 * 
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user1 name user1
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user2 name user2
 * 
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user1:files:17241709254077376921 name file1 path home/yrb/file1
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user1:files:17241709254077376922 name file2 path home/yrb/file2
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user1:files:17241709254077376923 name file3 path home/yrb/file3
 *
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user2:files:18241709254077376921 name file1 path home/yrb/user2/file1
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user2:files:18241709254077376922 name file2 path home/yrb/user2/file2
 * HMSET node:f526255265340d994510f8d1652e1eb1:user:user2:files:18241709254077376923 name file3 path home/yrb/user2/file3
 * 
 * HMGET node:f526255265340d994510f8d1652e1eb1:user:user1:files:17241709254077376921 name path
 */

struct action acs[] = {
    {.type = NODE_SET, .cmdline = "HMSET node:%s uuid %s ip %s mac %s", .exec = NULL, .syncexec = kx_set_reply},
    {.type = USER_REG, .cmdline = "HMSET node:%s:user:%s username %s password %s isonline %d", .exec = NULL, .syncexec = kx_set_reply},
    {.type = USER_LOGIN, .cmdline = "HMSET node:%s:user:%s isonline %d", .exec = NULL, .syncexec = kx_set_reply},
    {.type = USER_GET, .cmdline = "HGETALL node:%s:user:%s", .exec = NULL, .syncexec = NULL},
    {.type = NODE_GET, .cmdline = "HMSET node:%s", .exec = NULL, .syncexec = NULL},
    {.type = FILE_CRYPT, .cmdline = "HMSET node:%s:user:%s:files:%lu filename %s path %s uuid %lu", .exec = NULL, .syncexec = kx_set_reply},
    {.type = FILE_GET, .cmdline = "HGETALL %s", .exec = NULL, .syncexec = NULL},
    {.type = FILE_GETLIST, .cmdline = "KEYS node:%s:user:%s:files:*", .exec = NULL, .syncexec = NULL},
};

#define ACSIZE sizeof(acs)/sizeof(acs[0])

void kx_free_net(kxsyncnet *net) {
    redisFree(net->context);
    zfree(net);
}

struct action *kx_search_action(kxtype type) {
    struct action *ac = NULL;
    for (int i = 0; i < ACSIZE; i++) {
        if (acs[i].type == type)
            ac = &(acs[i]);
    }
    return ac;
}

kxsyncnet *kx_sync_creat_net(const char *addr, uint32_t port) {
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    kxsyncnet *net = zmalloc(sizeof(*net));
    if (net == NULL) return NULL;

    net->context = redisConnectWithTimeout(addr, port, timeout);
    if (net->context == NULL || net->context->err) {
        if (net->context) {
            printf("Connection error: %s\n", net->context->errstr);
            redisFree(net->context);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    return net;
}

void kx_sync_send_cmd(kxsyncnet *net, struct action *ac, const char *fmt, ...) {
    int                 size = 0;
    va_list             ap;
    char                *ptr;

    if (ac != NULL) {
        va_start(ap, fmt);
        size = vsnprintf(NULL, 0, fmt, ap);
        va_end(ap);

        if (size < 0) return;
        size++;

        ptr = zmalloc(size);
        if (ptr == NULL) return;

        va_start(ap, fmt);
        size = vsnprintf(ptr, size, fmt, ap);
        va_end(ap);

        if (size < 0) {
            free(ptr);
            return;
        }
        // printf("cmd : %s\n", ptr);
        kx_command(net->context, ptr, ac);

        zfree(ptr);
    }
}

static void kx_command(redisContext *c, const char *cmd, struct action *ac) {
    redisReply  *reply;
    reply = redisCommand(c, cmd);

    if (reply == NULL) {
        printf("%s error: %s\n", cmd, c->errstr ? c->errstr : "unknown error");
        return;
    }
    ac->syncexec(reply);
    freeReplyObject(reply);
}

static void kx_set_reply(redisReply *reply) {
    if (reply && reply->str) {
        printf("%s\n", reply->str);
    }
}
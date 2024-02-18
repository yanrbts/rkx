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
#include "rkxconfig.h"
#include "zmalloc.h"
#include "node.h"
#include "user.h"
#include "chex.h"

static void kx_user_creat_key(kxuser *user);

kxuser *kx_creat_user(const char *name, uint16_t sn, const char *pwd, uint16_t sp) {
    kxuser *u = zmalloc(sizeof(*u));
    if (u == NULL) return NULL;

    strncpy(u->username, name, sizeof(u->username)-1);
    u->username[sizeof(u->username)] = '\0';

    strncpy(u->pwd, pwd, sizeof(u->pwd)-1);
    u->pwd[sizeof(u->pwd)] = '\0';

    memset(u->trustid, 0, sizeof(u->trustid));
    u->isonline = 0;
    u->node = kx_get_node();
    kx_user_creat_key(u);

    return u;
}

void kx_free_user(kxuser *user) {
    if (user->key)
        zfree(user->key);
    zfree(user);
}

static void kx_user_creat_key(kxuser *user) {
    size_t len;
    char str[64] = {0};
    char hex[128] = {0};

    snprintf(str, sizeof(str), "%s%s", user->node->uuid, user->username);
    chex_encode(hex, sizeof(hex), str, strlen(str));

    len = strlen(hex) / 2;
    user->key = zmalloc(len * sizeof(uint8_t));

    for (size_t i = 0; i < len; i++) {
        sscanf(hex + 2 * i, "%02hhx", &user->key[i]);
    }
}
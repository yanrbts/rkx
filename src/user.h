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
#ifndef __KX_USER_H__
#define __KX_USER_H__

#include "node.h"

typedef struct kxuser {
    char username[32];  // user name
    char pwd[32];       // password
    char trustid[37];   // trust id
    int isonline;       // Is the user online sign? 1 online 0: offline
    kxnode *node;       // User associated node
    uint8_t *key;       // user key
} kxuser;

/** Create machine node information
 * 
 * @param name user name
 * @param sn user name length
 * @param pwd password
 * @param sp password length
 * @return return new user infomation 
 * @warning If the acquisition fails, return empty. 
 *          If the attribute is not acquired, return "unkown".
 */
kxuser *kx_creat_user(const char *name, uint16_t sn, const char *pwd, uint16_t sp);

/** Release user
 * 
 * @param user user object
 * @note user information is released only when the process exits 
 */
void kx_free_user(kxuser *user);

/** Get user key
 * 
 * @param user user object
 * @note Returns the user key, or null if failed
 */
const char *kx_user_get_key(kxuser *user);

#endif
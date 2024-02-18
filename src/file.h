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
#ifndef __FILE_H__
#define __FILE_H__

#include "rkx.h"

typedef enum filetype {
    KXCIPHER = 0x01,
    KXPLAIN,
} kxfiletype;

typedef struct kxfile {
    char fname[NAME_MAX];
    char fullname[PATH_MAX];
    uint64_t uuid;
    kxfiletype type;
} kxfile;

/** create file object
 * 
 * @param fname file path
 * @return return kxfile object and store it to list
 */
kxfile *kx_crypt_file(const char *fname);

/** decrypt file object
 * 
 * @param fname file path
 * @return Returns 0 on success and -1 on failure
 */
int kx_decrypt_file(const char *fname, const char *key);

/** free kxfile object
 * @note kxfile object pointer It is best not to be empty
 */
void kx_free_file(kxfile *kf);

/** Calculate file uuid
 * @param fname file path
 * @return return File calculated uuid 
 */
uint64_t kx_get_file_uuid(const char *fname);

#endif
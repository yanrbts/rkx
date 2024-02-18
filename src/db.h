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
#ifndef __KX_DB_H__
#define __KX_DB_H__

#include "rkxconfig.h"

#define KX_DB_INSERT_FILE   1
#define KX_DB_GET_FILE      2
#define KX_DB_GET_FILELIST  3

typedef struct kxdb {
    uint64_t max_mapsize; /* Set the size of the memory map to use for this environment. */
    MDB_env *env;
    MDB_dbi dbi;
    char dbname[32];       /* The name of the database to open. */
    char dbpath[128];      /* db file path */
} kxdb;

/** @brief Create a db object and currently use the lmdb 
 *         library to provide data persistence services.
 * @param[in] size Set the size of the memory map to use for this environment.
 * @param[in] dbpath db storage path
 * @param[in] dbname The name of the database to open.
 * @note The size should be a multiple of the OS page size. The default is
 *       10485760 bytes. Bytes as unit
 * @return return kxdb pointer, Returns NULL if failed */
kxdb *kx_creat_db(uint64_t size, const char *dbpath, const char *dbname);

/** @brief free kxdb object
 *         This function is usually called at the end of the program 
 * @param db kxdb object pointer */
void kx_free_db(kxdb *db);

/** @brief Store local data
 * @param[in] db kxdb object pointer 
 * @param[in] type store type @ref define
 * @param[in] key store key 
 * @param[in] data store value
 * @return Returns 0 on success, -1 otherwise */
int kx_store_db(kxdb *db, int type, void *key, void *data);

/** @brief Get db storage data, if key = NULL traverse all data, outdata = NULL.
 * @param[in] db kxdb object pointer 
 * @param[in] type store type @ref define
 * @param[in] key store key 
 * @param[out] outdata store get value
 * @return Returns 0 on success, -1 otherwise */
int kx_get_db(kxdb *db, int type, void *key, void **outdata);

#endif
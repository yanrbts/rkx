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
#include "db.h"
#include "zmalloc.h"
#include "file.h"
#include "util.h"

#define KXDEFAULTSIZE  (10 * 1024 * 1024) // 10M

#define MDB_CHECK(call)                             \
    do {                                            \
        int ret = call;                             \
        if (ret != MDB_SUCCESS) {                   \
            fprintf(stderr, "LMDB error: %s\n",     \
                    mdb_strerror(ret));             \
            return;                                 \
        }                                           \
    } while (0)

static void insert_file(kxdb *db, const char *ky, kxfile *file);
static void get_file(kxdb *db, void *key, kxfile **outfile);
static void get_file_list(kxdb *db);

kxdb *kx_creat_db(uint64_t size, const char *dbpath, const char *dbname) {
    int rc;
    struct stat st;

    kxdb *db = zmalloc(sizeof(*db));
    if (db == NULL) return NULL;

    db->max_mapsize = size;
    strncpy(db->dbpath, dbpath, sizeof(db->dbpath)-1);
    db->dbpath[sizeof(db->dbpath)] = '\0';
    strncpy(db->dbname, dbname, sizeof(db->dbname)-1);
    db->dbname[sizeof(db->dbname)] = '\0';
    /* Open LMDB environment */
    rc = mdb_env_create(&db->env);
    if (rc) {
		fprintf(stderr, "mdb_env_create failed, error %d %s\n", rc, mdb_strerror(rc));
		goto err;
	}

    rc = mdb_env_set_mapsize(db->env, size <= 0 ? KXDEFAULTSIZE : size);
    if (rc) {
		fprintf(stderr, "mdb_env_set_mapsize failed, error %d %s\n", rc, mdb_strerror(rc));
		goto err;
	}
    mdb_env_set_maxdbs(db->env, 4);

    /* Check if the directory exists and create it if it does not exist */
    if (stat(dbpath, &st) < 0) {
        kx_mkdirp(dbpath, 0777);
    }

    rc = mdb_env_open(db->env, dbpath, 0, 0664);
    if (rc) {
        fprintf(stderr, "mdb_env_open failed, error %d %s\n", rc, mdb_strerror(rc));
		goto err;
	}

    return db;
err:
    return NULL;
}

void kx_free_db(kxdb *db) {
    mdb_env_close(db->env);
    zfree(db);
}

int kx_store_db(kxdb *db, int type, void *key, void *data) {
    int ret = -1;

    switch (type) {
    case KX_DB_INSERT_FILE:
        insert_file(db, (const char*)key, (kxfile*)data);
        ret = 0;
        break;
    default:
        break;
    }
    
    return ret;
}

int kx_get_db(kxdb *db, int type, void *key, void **outdata) {
    int ret = -1;

    switch (type) {
    case KX_DB_GET_FILE:
        get_file(db, key, (kxfile**)outdata);
        ret = 0;
        break;
    case KX_DB_GET_FILELIST:
        get_file_list(db);
        ret = 0;
        break;
    default:
        break;
    }

    return ret;
}

static void insert_file(kxdb *db, const char *ky, kxfile *file) {
    MDB_dbi dbi; 
    MDB_txn *txn = NULL;
    MDB_val key, data;

    // Start a new transaction
    mdb_txn_begin(db->env, NULL, 0, &txn);
    mdb_dbi_open(txn, db->dbname, MDB_CREATE, &dbi);

    // Serialize the Person struct and set it as the data
    key.mv_size = strlen(ky);
    key.mv_data = &file->uuid;

    data.mv_size = sizeof(kxfile);
    data.mv_data = (void *)file;

    // Insert the serialized data into LMDB
    MDB_CHECK(mdb_put(txn, dbi, &key, &data, 0));
    // Commit the transaction
    MDB_CHECK(mdb_txn_commit(txn));
}

static void get_file(kxdb *db, void *key, kxfile **outfile) {
    MDB_txn *txn = NULL;
    MDB_cursor *cursor;
    MDB_val mdb_key, mdb_data;

    MDB_CHECK(mdb_txn_begin(db->env, NULL, MDB_RDONLY, &txn));
    MDB_CHECK(mdb_dbi_open(txn, db->dbname, 0, &db->dbi));
    MDB_CHECK(mdb_cursor_open(txn, db->dbi, &cursor));

    // Set the cursor position to the specified key
    mdb_key.mv_data = (void *)key;
    mdb_key.mv_size = sizeof(uint64_t);

    MDB_CHECK(mdb_cursor_get(cursor, &mdb_key, &mdb_data, MDB_SET));

    // Check if the key exists
    if (mdb_cursor_get(cursor, &mdb_key, &mdb_data, MDB_GET_CURRENT) == MDB_SUCCESS) {
        // Key exists, print the data
        *outfile = (kxfile *)mdb_data.mv_data;
    } else {
        *outfile = NULL;
        printf("Key not found: %lu\n", *(uint64_t*)key);
    }

    // Close the cursor and transaction
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
}

static void get_file_list(kxdb *db) {
    MDB_txn *txn = NULL;
    MDB_cursor *cursor;
    MDB_val mdb_key, mdb_data;
    int rc;

    rc = mdb_txn_begin(db->env, NULL, MDB_RDONLY, &txn);
    if (rc != MDB_SUCCESS) {
        fprintf(stderr, "Error: Failed to begin LMDB transaction (%s)\n", mdb_strerror(rc));
        return;
    }

    // Open the default database
    rc = mdb_dbi_open(txn, db->dbname, 0, &db->dbi);
    if (rc != MDB_SUCCESS) {
        fprintf(stderr, "Error: Failed to open LMDB database (%s)\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return;
    }

    // Open a cursor
    rc = mdb_cursor_open(txn, db->dbi, &cursor);
    if (rc != MDB_SUCCESS) {
        fprintf(stderr, "Error: Failed to open LMDB cursor (%s)\n", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return;
    }

    // Iterate through all data
    while (mdb_cursor_get(cursor, &mdb_key, &mdb_data, MDB_NEXT) == MDB_SUCCESS) {
        char *key = (char *)mdb_key.mv_data;
        kxfile *kf = (kxfile *)mdb_data.mv_data;
        printf(" [*] %-10s%-30s%20lu [L+]\n", kf->fname, kf->fullname, kf->uuid);
    }
    // Close the cursor and transaction
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
}
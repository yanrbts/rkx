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

#include "file.h"

#define AES_BLOCK_SIZE  16
#define MAXFILESIZE     (100 * 1024 * 1024)  /* Process files smaller than 100M */

static uint64_t calculate_xxhash(const char *file_path) {
    FILE            *fp;
    XXH64_hash_t    hash;
    char            *buffer;
    uint64_t        file_size;
    

    fp = fopen(file_path, "rb");
    if (!fp) {
        perror("Error opening file");
        goto err;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (char *)zmalloc(file_size);
    if (buffer == NULL) {
        perror("Error allocating memory");
        goto err;
    }

    if (fread(buffer, 1, file_size, fp) != file_size) {
        perror("Error reading file");
        goto err;
    }
    fclose(fp);

    hash = XXH64(buffer, file_size, 0);
    zfree(buffer);
    return hash;
err:
    return 0;
}

/* Encrypting and decrypting files is currently stuck when testing 100M files, 
 * which takes too long. It is about the same for 10M files and needs to be 
 * further optimized. It cannot be used for files that are too large, 
 * otherwise it will get stuck. */
static int encrypt_file(const char *filename, const char *key) {
    FILE *fp;
    size_t fileSize;
    size_t numBlocks;
    struct AES_ctx ctx;

    fp = fopen(filename, "rb+");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Calculate the number of blocks that need to be processed
    numBlocks = (fileSize + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;

    // Initialize AES context
    AES_init_ctx(&ctx, (const uint8_t *)key);

    // block by block encryption
    uint8_t buffer[AES_BLOCK_SIZE];
    for (size_t block = 0; block < numBlocks; ++block) {
        // Read block data
        size_t bytesRead = fread(buffer, 1, AES_BLOCK_SIZE, fp);

        // Fill last block when needed
        if (bytesRead < AES_BLOCK_SIZE) {
            memset(buffer + bytesRead, AES_BLOCK_SIZE - bytesRead, AES_BLOCK_SIZE - bytesRead);
        }

        // Encrypted block
        AES_ECB_encrypt(&ctx, buffer);

        // Write encrypted blocks back to file
        fseek(fp, block * AES_BLOCK_SIZE, SEEK_SET);
        fwrite(buffer, 1, AES_BLOCK_SIZE, fp);
    }

    fclose(fp);
    return 0;
}

static int decrypt_file(const char *filename, const char *key) {
    FILE *fp;
    struct AES_ctx ctx;
    size_t numBlocks;

    fp = fopen(filename, "rb+");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Calculate the number of blocks that need to be processed
    numBlocks = (fileSize + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;

    // Initialize AES context
    AES_init_ctx(&ctx, (const uint8_t *)key);

    // block by block decryption
    uint8_t buffer[AES_BLOCK_SIZE];
    for (size_t block = 0; block < numBlocks; ++block) {
        // Read block data
        size_t bytesRead = fread(buffer, 1, AES_BLOCK_SIZE, fp);

        // Fill last block when needed
        if (bytesRead < AES_BLOCK_SIZE) {
            memset(buffer + bytesRead, AES_BLOCK_SIZE - bytesRead, AES_BLOCK_SIZE - bytesRead);
        }

        AES_ECB_decrypt(&ctx, buffer);

        // Write encrypted blocks back to file
        fseek(fp, block * AES_BLOCK_SIZE, SEEK_SET);
        fwrite(buffer, 1, AES_BLOCK_SIZE, fp);
    }

    fclose(fp);
    return 0;
}

kxfile *kx_crypt_file(const char *fname) {
    char *name;
    kxfile *kf = NULL;
    struct stat st;
    /* We are currently processing files that are less than 100 MB. 
     * Files larger than 100 MB are not being processed at the moment 
     * to prevent potential performance issues or system slowdowns.*/
    if (stat(fname, &st) == -1 || st.st_size > MAXFILESIZE) {
        if (st.st_size > MAXFILESIZE)
            fprintf(stderr, "File size > 100M Exceeding the file size limit or File does not exist.\n");
        else 
            perror("Error stat() failed");
        goto err;
    }

    kf = zmalloc(sizeof(*kf));
    if (kf == NULL)
        goto err;

    if (encrypt_file(fname, client.user->key) == -1)
        goto err;

    kf->uuid = calculate_xxhash(fname);
    if (kf->uuid == 0)
        goto err;

    strncpy(kf->fullname, fname, sizeof(kf->fullname));
    name = basename((char*)fname);
    strncpy(kf->fname, name, sizeof(kf->fname));
    kf->type = KXCIPHER;
    
    return kf;
err:
    if (kf) zfree(kf);
    return NULL;
}

int kx_decrypt_file(const char *fname, const char *key) {
    return decrypt_file(fname, key);
}

void kx_free_file(kxfile *kf) {
    zfree(kf);
}

uint64_t kx_get_file_uuid(const char *fname) {
    return calculate_xxhash(fname);
}
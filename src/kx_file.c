/*
 * Copyright 2023-2024 yanruibinghxu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "kx_file.h"

#define AUTHORS             "Written by Yan Ruibing."
#define PACKAGE_VERSION     "0.0.1"

struct state {
    bool isecrypt;
    bool isdecrypt;
    bool istrace;
    bool isgetlist;
    char *file;
};

static void kx_filelist_reply(redisReply *reply);
static void kx_file_reply(redisReply *reply);
static void kx_local_cryptfilelist();

static struct state *state = NULL;
static struct option const long_options[] = {
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}
};

static void usage() {
    printf ("Usage: file [OPTION]... \n"
                "list file interface contents\n\n"
                "  -e,              encrypt documents .\n"
                "  -d,              File decryption .\n"
                "  -t,              Document traceability .\n"
                "  -l,              Query file list .\n"
                "      --help       display this help and exit\n"
                "      --version    output version information and exit\n\n"
                "Examples:\n"
                "  file -e filename\n"
                "  file -d filename\n\n");
}

/**
 * Parses command line flags into a global application state.
 */
static int
parse_options (int argc, char *argv[]) {
    int ret = -1;
    int opt = 0;
    int option_index = 0;

    if (argv[argc-1] == NULL || argv[argc-1][0] == '\0') {
        fprintf(stderr, "Invalid command line arguments\n");
        goto err;
    }

    optind = 0;
    while (true) {
        opt = getopt_long(argc, argv, "e:d:t:lhv", long_options, &option_index);

        if (opt == -1) break;

        switch (opt) {
        case 'e':
            if (state->isdecrypt || state->istrace) {
                fprintf(stderr, "Invalid command line arguments\n");
                goto err;
            }
            if (optarg) {
                state->file = strdup(optarg);
                state->isecrypt = true;
                ret = 0;
            }
            break;
        case 'd':
            if (state->isecrypt || state->istrace) {
                fprintf(stderr, "Invalid command line arguments\n");
                goto err;
            }
            if (optarg) {
                state->file = strdup(optarg);
                state->isdecrypt = true;
                ret = 0;
            }
            break;
        case 't':
            if (state->isecrypt || state->isdecrypt) {
                fprintf(stderr, "Invalid command line arguments\n");
                goto err;
            }
            if (optarg) {
                state->file = strdup(optarg);
                state->istrace = true;
                ret = 0;
            }
            break;
        case 'l':
            state->isgetlist = true;
            ret = 0;
            goto out;
        case 'v':
            printf ("%s (%s) %s\n", argv[0], PACKAGE_VERSION, AUTHORS);
            ret = -2;
            goto out;
        case 'h':
            usage();
            ret = -2;
            goto out;
        default: goto err;
        }
    }

    if ((argc - option_index) < 2) {
        error(0, 0, "missing operand");
        goto err;
    } else {
        goto out;
    }
err:
    error(0, 0, "Try user --help for more information.");
out:
    return ret;
}

/**
 * Initializes the global application state.
 */
static struct state*
init_state()
{
    struct state *state = zmalloc(sizeof (*state));
    if (state == NULL)
        goto out;
    
    state->isdecrypt = false;
    state->isecrypt = false;
    state->istrace = false;
    state->isgetlist = false;
    state->file = NULL;
out:
    return state;
}

static void free_state() {
    if (state) {
        if (state->file) free(state->file);
        free(state);
        state = NULL;
    }
}

static int file_encrypt() {
    kxfile *kf;
    redisReply *reply;
    char buf[64] = {0};

    if (state->file == NULL) {
        fprintf(stderr, "Error file name is NULL\n");
        return -1;
    }

    kf = kx_crypt_file(state->file);
    if (kf) {
        listAddNodeHead(client.local_cryptfiles, kf);

        struct action *ac = kx_search_action(FILE_CRYPT);
        kx_sync_send_cmd(client.net, ac, ac->cmdline, 
                        kf->uuid,
                        kf->fname,
                        kf->fullname,
                        kf->uuid,
                        client.user->username,
                        client.node->uuid);

        /* Save encrypted file information and make local persistence*/
        snprintf(buf, sizeof(buf), "%s:%lu", client.user->username, kf->uuid);
        kx_store_db(client.db, KX_DB_INSERT_FILE, (void*)buf, (void*)kf);
    } else {
        fprintf(stderr, "Error crypt file failed.\n");
        return -1;
    }
    return 0;
}

static int file_decrypt() {
    int ret = -1;

    if (state->file == NULL) {
        fprintf(stderr, "Error file name is NULL\n");
        return -1;
    }

    ret = kx_decrypt_file(state->file, client.user->key);
    if (ret != 0) {
        fprintf(stderr, "Decryption of file failed\n");
        return -1;
    } else {
        printf("Decryption of file success\n");
    }
    return 0; 
}

static int file_getfilelist() {
    // kx_local_cryptfilelist();
    kx_get_db(client.db, KX_DB_GET_FILELIST, NULL, NULL);
}

int do_file(struct context *ctx) {
    int ret = -1;
    int argc = ctx->argc;
    char **argv = ctx->argv;

    state = init_state();
    if (state == NULL) {
        error(0, errno, "failed to initialize state");
        goto out;
    }

    ret = parse_options(argc, argv);
    switch (ret) {
        case -2: ret = 0;
        case -1: goto out;
    }
    if (state->isecrypt)
        file_encrypt();
    else if (state->isgetlist)
        file_getfilelist();
    else if (state->isdecrypt)
        file_decrypt();
out:
    free_state();
    return ret;
}

static void kx_filelist_reply(redisReply *reply) {
    struct action *ac;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
        for (size_t i = 0; i < reply->elements; i++) {
            ac = kx_search_action(FILE_GET);
            ac->syncexec = kx_file_reply;
            kx_sync_send_cmd(client.net, ac, ac->cmdline, 
                            reply->element[i]->str);
        }
        printf("%88s numbers: %ld\n", "", reply->elements);
    }
}

static void kx_file_reply(redisReply *reply) {
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i += 2) {
            if (strcmp(reply->element[i]->str, "filename") == 0) {
                printf(" [*] %-24s ", reply->element[i+1]->str);
            } else if (strcmp(reply->element[i]->str, "path") == 0) {
                printf(" %-64s [L+]", reply->element[i+1]->str);
            }
        }
        printf("\n");
    }
}

static void kx_local_cryptfilelist() {
    listNode *node, *nextnode;

    node = listFirst(client.local_cryptfiles);
    while (node) {
        kxfile *kf = (kxfile *)node->value;
        printf(" [*] %-24s%-64s[L+]\n", kf->fname, kf->fullname);
        nextnode = listNextNode(node);
        node = nextnode;
    }
}
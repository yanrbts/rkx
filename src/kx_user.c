/*
 * Copyright 2023-2023 yanruibinghxu
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
#include "kx_user.h"

#define AUTHORS             "Written by Yan Ruibing."
#define PACKAGE_VERSION     "0.0.1"

struct state {
    bool isreg;
    bool isget;
    char *puser;
    char *ppwd;
};

static void free_state();
static void kx_user_reply(redisReply *reply);

static struct state *state = NULL;
static struct option const long_options[] = {
    {"register", no_argument, NULL, 'r'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}
};

static void usage() {
    printf ("Usage: user [OPTION]... \n"
                "list user interface contents\n\n"
                "  -u,              user name .\n"
                "  -p,              password .\n"
                "  -g,              Get user infomation .\n"
                "  -r, --register   User register .\n"
                "      --help       display this help and exit\n"
                "      --version    output version information and exit\n\n"
                "Examples:\n"
                "  user -r -u admin -p pwd\n\n");
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
        opt = getopt_long(argc, argv, "u:p:grhv", long_options, &option_index);
        
        if (opt == -1) break;

        switch (opt) {
            case 'u':
                if (optarg) {
                    state->puser = strdup(optarg);
                    ret = 0;
                    break;
                } else {
                    ret = -2;
                    break;
                }
            case 'p':
                if (optarg) {
                    state->ppwd = strdup(optarg);
                    ret = 0;
                    break;
                } else {
                    ret = -2;
                    break;
                }
            case 'r': 
                state->isreg = true;
                ret = 0;
                break;
            case 'g':
                state->isget = true;
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

    if (state->isreg) {
        if (state->puser == NULL || state->ppwd == NULL) {
            fprintf(stderr, "Options -u and -p require arguments\n");
            ret = -2;
            goto err;
        }
    } else {
        if ((state->puser && state->ppwd == NULL)
            || (state->puser == NULL && state->ppwd)) {
            fprintf(stderr, "Options -u and -p require arguments\n");
            ret = -2;
            goto err;
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
    
    state->isget = false;
    state->isreg = false;
    state->puser = NULL;
    state->ppwd = NULL;
out:
    return state;
}

static void free_state() {
    if (state) {
        if (state->puser) free(state->puser);
        if (state->ppwd) free(state->ppwd);
        free(state);
        state = NULL;
    }
}

static int kx_user_register() {
    client.user = kx_creat_user(state->puser, strlen(state->puser), state->ppwd, strlen(state->ppwd));
    client.user->isonline = 1;
    client.db = kx_creat_db(MAXMAPSIZE, "./data", state->puser);
    struct action *ac = kx_search_action(USER_REG);
    kx_sync_send_cmd(client.net, ac, ac->cmdline,
                client.node->uuid,
                client.user->username,
                client.user->username, 
                client.user->pwd, 1);
    return 0;
}

static int kx_user_get() {
    struct action *ac = kx_search_action(USER_GET);
    ac->syncexec = kx_user_reply;
    kx_sync_send_cmd(client.net, ac, ac->cmdline, client.node->uuid, client.user->username);
    return 0;
}

static int kx_node_set() {
    struct action *ac = kx_search_action(NODE_SET);
    kx_sync_send_cmd(client.net, ac, ac->cmdline, 
                    client.node->uuid,
                    client.node->uuid,
                    client.node->ip,
                    client.node->mac);
    return 0;
}

int do_user(struct context *ctx) {
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
    if (state->isreg) {
        ret = kx_node_set();
        ret = kx_user_register();
    } else if (state->isget) {
        ret = kx_user_get();
    }
    
out:
    free_state();
    return ret;
}

static void kx_user_reply(redisReply *reply) {
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i += 2) {
            if (strcmp(reply->element[i]->str, "username") == 0) {
                printf("username: %s ", reply->element[i+1]->str);
            } else if (strcmp(reply->element[i]->str, "password") == 0) {
                printf("password: %s ", reply->element[i+1]->str);
            } else if (strcmp(reply->element[i]->str, "isonline") == 0) {
                printf("isonline: %d ", atoi(reply->element[i+1]->str));
            }
        }
        printf("\n");
    }
}
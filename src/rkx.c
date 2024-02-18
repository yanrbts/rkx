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
#include "rkx.h"
#include "linenoise.h"
#include "kx_user.h"
#include "kx_file.h"
#include "kx_command.h"

#define USERNAME    "yrb"
#define USERPWD     "123"
#define KX_PROMPT   "rkx>"

struct cmd {
    char *name;
    int (*execute)(struct context *ctx);
};

struct kxclient client;
struct context *gctx;
char *kx_prompt = NULL;

static const char usage[]
    = "\n"
    "    __              __\n"      
    "   / /___  ____  __/ /__      Server ip: %s\n"
    "  / //_/ |/_/ / / / //_/      Author   : %s\n"
    " / ,< _>  </ /_/ / ,<         Version  : 0.0.1\n"  
    "/_/|_/_/|_|\\__, /_/|_|\n"  
    "          /____/  \n\n";

static int shell_usage() {
    printf ("The following commands are supported:\n"
            " * user\n"
            " * file\n"
            " * ls\n"
            " * cd\n"
            " * help\n"
            " * mkdir\n"
            " * touch\n"
            " * quit\n"
            " * rm\n"
            " * stat\n"
            " * tail\n"
            " * truncate\n"
            " * clear\n"
            " * flock\n"
            " * mv\n");
    return 0;
}


static struct cmd const cmds[] =
{
    {.name = "user", .execute = do_user},
    {.name = "file", .execute = do_file},
    {.name = "ls", .execute = do_command},
    {.name = "cd", .execute = do_command},
    {.name = "help", .execute = shell_usage}
};
#define NUM_CMDS sizeof(cmds) / sizeof(cmds[0])

static const struct cmd* get_cmd(char *name) {
    const struct cmd *cmd = NULL;
    for (int j = 0; j < NUM_CMDS; j++) {
        if (strcmp (name, cmds[j].name) == 0) {
            cmd = &(cmds[j]);
            break;
        }
    }
    return cmd;
}

static int split_str(char *line, char ***argv) {
    int argc = 0;
    char *line_start = line;

    while (*line != '\0') {
        while (*line == ' ') {
            line++;
        }

        if (*line != '\0') {
            argc++;
        }

        while (*line != ' ' && *line != '\n' && *line != '\0') {
            line++;
        }
    }

    *argv = (char **)malloc(sizeof(char *) * argc);
    if (*argv == NULL) {
        goto out;
    }

    int cur_arg = 0;
    line = line_start;

    while (cur_arg < argc && *line != '\0') {
        while (*line == ' ') {
            line++;
        }

        if (*line != '\0') {
            (*argv)[cur_arg++] = line;
        }

        while (*line != ' ' && *line != '\n' && *line != '\0') {
            line++;
        }

        if (*line != '\0') {
            *line = '\0';
            line++;
        }
    }

out:
    return argc;
}

static void  
do_completion(char const *prefix, linenoiseCompletions* lc) {

}

static void kx_loop() {
    char            *line;
    const char      *file = "./history";
    kx_prompt        = KX_PROMPT;
    const struct cmd *cmd;

    linenoiseHistoryLoad(file);
    linenoiseSetCompletionCallback(do_completion);

    while ((line = linenoise(kx_prompt)) != NULL) {
        if (line[0] != '\0' && line[0] != '/') {
            gctx->argc = split_str(line, &gctx->argv);
            if (gctx->argc == 0) {
                continue;
            }
            cmd = get_cmd(gctx->argv[0]);
            if (cmd) {
                cmd->execute(gctx);
            }
            free(gctx->argv);
            gctx->argv = NULL;
        } else {

        }
        free(line);
    }

    linenoiseHistorySave(file);
}

void rkx_init(struct kxclient *kx) {
    kx->local_cryptfiles = listCreate();
    kx->remote_cryptfiles = listCreate();
    kx->node = kx_creat_node();
    kx->user = NULL;
    kx->net = kx_sync_creat_net("127.0.0.1", 6379);
    kx->db = NULL;
    kx->mq = kx_create_mq();

    pthread_rwlock_init(&kx->rwlock, NULL);
}

int main(int argc, char *argv[]) {
    printf(usage, "127.0.0.1", "Yan RuiBing");
    setlocale(LC_COLLATE,"");

    /* init client struct data*/
    rkx_init(&client);

    gctx = zmalloc(sizeof (*gctx));
    if (gctx == NULL) {
        log_error("failed to initialize context");
    }
    gctx->argc = 0;
    gctx->argv = NULL;
    kx_loop();

    kx_free_net(client.net);
    pthread_rwlock_destroy(&client.rwlock);
    free(gctx->argv);
    zfree(gctx);
    return 0;
}
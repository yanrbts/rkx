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
#include "kx_command.h"

#define AUTHORS             "Written by Yan Ruibing."
#define PACKAGE_VERSION     "0.0.1"

int do_command(struct context *ctx) {
    int ret = -1;
    int total_length = 0;
    int offset = 0;
    int argc = ctx->argc;
    char **argv = ctx->argv;
    
    for (int i = 0; i < argc; ++i) {
        total_length += snprintf(NULL, 0, "%s ", argv[i]);
    }

    char command[total_length + 1];

    for (int i = 0; i < argc; ++i) {
        offset += snprintf(command + offset, sizeof(command) - offset, "%s ", argv[i]);
    }
    command[offset - 1] = '\0';

    system(command);

    if (strcmp(argv[0], "cd") == 0)
        chdir(argv[1]);

    return 0;
out:
    return ret;
}
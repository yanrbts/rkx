#include <stdlib.h>
#include <stdio.h>
#include <hiredis/hiredis.h>

int main(int argc, char **argv) {
    redisContext *context = redisConnect("127.0.0.1", 6379);

    if (context == NULL || context->err) {
        if (context) {
            printf("Error: %s\n", context->errstr);
            redisFree(context);
        } else {
            printf("Unable to allocate Redis context\n");
        }
        return EXIT_FAILURE;
    }
    char *fmt = "HMSET node:f526255265340d994510f8d1652e1eb1:user:yrb:files:1646407530672067061%d"
                " filename thredis-master%d.zip path /home/yrb/src/thredis-master%d.zip"
                " uuid 1646407530672067061%d";
    char buf[1024] = {0};
    for (int i = 0; i < 100000; i++) {
        snprintf(buf, sizeof(buf), fmt, i, i, i, i);
        redisReply *reply = redisCommand(context, buf);

        freeReplyObject(reply);
    }
    
    redisFree(context);

    return EXIT_SUCCESS;
}
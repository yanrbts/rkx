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
#ifndef __KX_MQ__
#define __KX_MQ__

#include "rkxconfig.h"

typedef struct kxmq {
    struct mosquitto *mosq;
    char *topic;
} kxmq;

kxmq *kx_init_mq(const char *tp);

#endif
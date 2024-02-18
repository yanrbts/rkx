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
#ifndef __KX_MQ_H__
#define __KX_MQ_H__

#include "rkxconfig.h"

typedef void (*callback_connect)(struct mosquitto *mosq, void *obj, 
								int reason_code);
typedef void (*callback_subscribe)(struct mosquitto *mosq, void *obj, 
								int mid, int qos_count, const int *granted_qos);
typedef void (*callback_message)(struct mosquitto *mosq, void *obj, 
								const struct mosquitto_message *msg);
typedef void (*callback_log)(struct mosquitto *mosq, void *obj, 
							int level, const char *str);

typedef struct kxmq {
    struct mosquitto *mosq;
    int keepalive;				/* in seconds */
	char *host;
	int port;
    char *topic;
	callback_connect on_connect;
	callback_subscribe on_subscribe;
	callback_message on_message;
	callback_log on_log;
} kxmq;

/** @brief Create an MQTT management object. 
 * @param host Link server address
 * @param port Server port
 * @param tp Subscribe to the topic
 * @return kxmq MQTT manager object If it fails, return NULL.
*/
kxmq *kx_mq_init(const char *host, int port, const char *tp);

/** @brief Release MQTT management resources
 * @param kq mq instance.
*/
void kx_mq_free(kxmq *kq);

/** @brief Start mosquitto loop
 * @param kq kxmq instance.
 * @return If successful, it returns 0; otherwise, it returns -1. */
int kx_mq_loop_start(kxmq *kq);

void kx_mq_set_connect_cb(kxmq *kq, callback_connect fconnect);
void kx_mq_set_subscribe_cb(kxmq *kq, callback_subscribe fsubscribe);
void kx_mq_set_message_cb(kxmq *kq, callback_message fmessage);
void kx_mq_set_log_cb(kxmq *kq, callback_log flog);

#endif
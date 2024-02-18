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
#ifndef __KX_NODE_H__
#define __KX_NODE_H__

#define MAX_IP_LENGTH       16
#define MAX_MAC_LENGTH      18
#define MAX_UUID_LENGTH     33

typedef struct kxnode {
    char ip[MAX_IP_LENGTH];         // A string that stores the IP address, such as "192.168.1.1"  
    char mac[MAX_MAC_LENGTH];       // A string that stores the MAC address, such as "00:1A:2B:3C:4D:5E"
    char uuid[MAX_UUID_LENGTH];     /* A string that stores the UUID, such as 
                                     * "550e8400-e29b-41d4-a716-446655440000" */
} kxnode;

/** Create machine node information
 * 
 * @return return new node infomation 
 * @warning If the acquisition fails, return empty. 
 *          If the attribute is not acquired, return "unkown".
 */
kxnode *kx_creat_node(void);

/** Create machine node information
 * 
 * @return return current node infomation 
 * @warning If the acquisition fails, return NULL.
 *          It means that the node has not been created yet. 
 *          You need to refer to the @ref kx_creat_node function.
 */
kxnode *kx_get_node(void);

/** Release node
 * @note Node information is released only when the process exits 
 */
void kx_free_node(kxnode *node);

#endif
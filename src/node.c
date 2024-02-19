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
#include "rkxconfig.h"
#include "zmalloc.h"
#include "node.h"

#define MACHINEID "/etc/machine-id"

static kxnode *gnode = NULL;

static int kx_get_ipaddr(char *ip, size_t size) {
    int 			ret = -1;
	struct ifaddrs *ifaddr, *ifa;
    int 			family, s;
    char 			host[NI_MAXHOST];

	/* The  getifaddrs()  function  creates a linked list of structures 
	 * describing the network interfaces of the local system, and stores
     * the address of the first item of the list in *ifap.*/
	if (getifaddrs(&ifaddr) == -1) {
		strncpy(ip, "unknow", size-1);
        ip[size] = '\0';
		return 0;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr != NULL) {
			family = ifa->ifa_addr->sa_family;
			if (family == AF_INET) {
                s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, 
					NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
					strncpy(ip, "unknow", size-1);
                    ip[size] = '\0';
					goto ret;
                }

                if (strncmp(host, "127.", 4) != 0) {
                    //printf("%s: %s\n", ifa->ifa_name, host);
					strncpy(ip, host, size-1);
                    ip[size] = '\0';
					goto ret;
                }
            }
		}
	}
	strncpy(ip, "unknow", size - 1);
    ip[size] = '\0';
ret:
    freeifaddrs(ifaddr);
    return 0;
}

/* Get the mac address of the first physical network card, 
 * if successful, return 0, otherwise return -1 */
static int kx_get_mac(char *mac, size_t size) {
	int 			sock;
    struct ifreq 	ifr;
    unsigned char 	mac_address[6]; // MAC address is usually 6 bytes

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		snprintf(mac, size, "%s", "unkown");
		return 0;
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        close(sock);
		snprintf(mac, size, "%s", "unkown");
		return 0;
    }

	memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
	snprintf(mac, size, "%02X:%02X:%02X:%02X:%02X:%02X", 
			mac_address[0], mac_address[1], mac_address[2],
            mac_address[3], mac_address[4], mac_address[5]);
	close(sock);

	return 0;
}

static int kx_get_sysuuid(char *uuid, size_t size) {
    int     ret = -1;
    FILE    *fp;
    size_t  bytes;

    fp = fopen(MACHINEID, "r");
    if (fp == NULL) {
        strncpy(uuid, "unknow", size-1);
        uuid[size] = '\0';
        goto err;
    }
    bytes = fread(uuid, 1, size-1, fp);
    if (bytes > 0) {
        uuid[bytes] = '\0';
        ret = 0;
    } else {
        strncpy(uuid, "unknow", size-1);
        uuid[size] = '\0';
    }
err:
    return ret;
}

kxnode *kx_creat_node(void) {
    gnode = zmalloc(sizeof(*gnode));
    if (gnode == NULL) return NULL;

    kx_get_ipaddr(gnode->ip, sizeof(gnode->ip));
    kx_get_mac(gnode->mac, sizeof(gnode->mac));
    kx_get_sysuuid(gnode->uuid, sizeof(gnode->uuid));

    return gnode;
}

kxnode *kx_get_node(void) {
    return gnode;
}

void kx_free_node(kxnode *node) {
    zfree(node);
}
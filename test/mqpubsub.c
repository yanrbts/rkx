#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <mosquitto_broker.h>

#define HOST "localhost"
#define PORT 1883
#define TOPIC "test/topic"
#define QOS 1

// 保存客户端 ID 的全局变量
static char client_id[30];

// 消息回调函数，处理接收到的消息
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    // 获取客户端 ID
    char *client_id = (char *)userdata;

    // 如果消息来源不是自己，则处理消息
    if (strcmp(client_id, mosquitto_client_id(mosq)) != 0) {
        if (message->payloadlen) {
            printf("Received message on topic %s: %s\n", message->topic, (char *)message->payload);
        } else {
            printf("Received empty message on topic %s\n", message->topic);
        }
    }
}

int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    // 初始化 Mosquitto 库
    mosquitto_lib_init();

    // 生成一个随机的客户端 ID
    sprintf(client_id, "client_%d", rand() % 1000);

    // 创建 Mosquitto 客户端
    mosq = mosquitto_new(client_id, true, client_id);
    if (!mosq) {
        fprintf(stderr, "Error: Unable to create Mosquitto instance.\n");
        return 1;
    }

    // 设置消息回调函数
    mosquitto_message_callback_set(mosq, on_message);

    // 连接到 MQTT 代理
    rc = mosquitto_connect(mosq, HOST, PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Unable to connect to MQTT broker. Return code: %d\n", rc);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    // 订阅主题
    rc = mosquitto_subscribe(mosq, NULL, TOPIC, QOS);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Unable to subscribe to topic %s. Return code: %d\n", TOPIC, rc);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    // 发布消息
    rc = mosquitto_publish(mosq, NULL, TOPIC, 13, "Hello, Mosquitto!", QOS, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Unable to publish message to topic %s. Return code: %d\n", TOPIC, rc);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    // 循环处理 MQTT 消息
    rc = mosquitto_loop_forever(mosq, -1, 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Unable to start Mosquitto loop. Return code: %d\n", rc);
    }

    // 断开连接并清理资源
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

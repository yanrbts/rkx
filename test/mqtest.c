#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(A) (void)(A)

typedef void (*callback_connect)(struct mosquitto *mosq, void *obj, 
								int reason_code);
typedef void (*callback_subscribe)(struct mosquitto *mosq, void *obj, 
								int mid, int qos_count, const int *granted_qos);
typedef void (*callback_message)(struct mosquitto *mosq, void *obj, 
								const struct mosquitto_message *msg);
typedef void (*callback_log)(struct mosquitto *mosq, void *obj, 
							int level, const char *str);
typedef struct kxmq {
    struct mosquitto *mosq;     /* mosquitto client instance. */
    int keepalive;				/* in seconds */
	char *host;
	int port;
    char *topic;
	callback_connect on_connect;
	callback_subscribe on_subscribe;
	callback_message on_message;
	callback_log on_log;
} kxmq;

static kxmq *glmq = NULL;

/* Callback called when the client receives a CONNACK message from the broker. */
static void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    int rc;
	kxmq *kq = (kxmq*)obj;
    /* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* Making subscriptions in the on_connect() callback means that if the
	 * connection drops and is automatically resumed by the client, then the
	 * subscriptions will be recreated when the client reconnects. */
	rc = mosquitto_subscribe(mosq, NULL, kq->topic, 1);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}
}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
static void on_subscribe(struct mosquitto *mosq, void *obj, 
	int mid, int qos_count, const int *granted_qos) {
	
	int i;
	bool have_subscription = false;

	/* In this example we only subscribe to a single topic at once, but a
	 * SUBSCRIBE can contain many topics at once, so this is one way to check
	 * them all. */
	for(i = 0; i < qos_count; i++){
		printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		/* The broker rejected all of our subscriptions, we know we only sent
		 * the one SUBSCRIBE, so there is no point remaining connected. */
		fprintf(stderr, "Error: All subscriptions rejected.\n");
		mosquitto_disconnect(mosq);
	}
}

/* Callback called when the client receives a message. */
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	/* This blindly prints the payload, but the payload can be anything so take care. */
	printf("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
}

static void on_logcb(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(level);

	printf("%s\n", str);
}

/** @brief Create an MQTT management object. 
 * @param host Link server address
 * @param port Server port
 * @param tp Subscribe to the topic
 * @return kxmq MQTT manager object If it fails, return NULL.
*/
kxmq *kx_mq_init(const char *host, int port, const char *tp) {
	kxmq *kq = malloc(sizeof(*kq));

	if (kq == NULL)
		goto err;

	/* Must be called before any other mosquitto functions.
	 * This function is not thread safe.*/
	mosquitto_lib_init();

	/* Create a new client instance.
	 * id = NULL -> ask the broker to generate a client id for us
	 * clean session = true -> the broker should remove old sessions when we connect
	 * obj = NULL -> we aren't passing any of our private data for callbacks */
	kq->mosq = mosquitto_new(NULL, true, (void*)kq);
	if (kq->mosq == NULL) {
		fprintf(stderr, "Error: Out of memory.\n");
		goto err;
	}
	kq->keepalive = 60; /* 60 second */
	kq->host = strdup(host);
	kq->topic = strdup(tp);
	kq->port = port;

	kq->on_connect = NULL;
	kq->on_message = NULL;
	kq->on_subscribe = NULL;
	kq->on_log = NULL;

	return kq;

err:
	if (kq) free(kq);
	return NULL;
}

/** @brief Release MQTT management resources
 * @param kq mq instance.
*/
void kx_mq_free(kxmq *kq) {
	if (kq) {
		/* Use to free memory associated with a 
		 * mosquitto client instance.*/
		mosquitto_destroy(kq->mosq);

		/* Call to free resources associated with the library. */
		mosquitto_lib_cleanup();

		free(kq->host);
		free(kq->topic);
		free(kq);
	}
}

/** @brief Start mosquitto loop
 * @param kq kxmq instance.
 * @return If successful, it returns 0; otherwise, it returns -1. */
int kx_mq_loop_start(kxmq *kq) {
	int rc;

	/* Connect to host on port, with a keepalive of 'kq->keepalive' seconds.
	 * This call makes the socket connection only, it does not complete the MQTT
	 * CONNECT/CONNACK flow, you should use mosquitto_loop_start() or
	 * mosquitto_loop_forever() for processing net traffic. */
	rc = mosquitto_connect(kq->mosq, kq->host, kq->port, kq->keepalive);
	if(rc != MOSQ_ERR_SUCCESS) {
		/* Use to free memory associated with a mosquitto client instance. */
		mosquitto_destroy(kq->mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return -1;
	}

	/* Run the network loop in a blocking call.
	 *
	 * This call will continue forever, carrying automatic reconnections if
	 * necessary, until the user calls mosquitto_disconnect().
	 */
	mosquitto_loop_forever(kq->mosq, -1, 1);

	return 0;
}

void kx_mq_set_connect_cb(kxmq *kq, callback_connect fconnect) {
	kq->on_connect = fconnect != NULL ? fconnect : on_connect;
	mosquitto_connect_callback_set(kq->mosq, fconnect);
}

void kx_mq_set_subscribe_cb(kxmq *kq, callback_subscribe fsubscribe) {
	mosquitto_subscribe_callback_set(kq->mosq, fsubscribe);
}

void kx_mq_set_message_cb(kxmq *kq, callback_message fmessage) {
	mosquitto_message_callback_set(kq->mosq, fmessage);
}

void kx_mq_set_log_cb(kxmq *kq, callback_log flog) {
	mosquitto_log_callback_set(kq->mosq, flog);
}

int main(int argc, char *argv[]) {

	kxmq *mq = kx_mq_init("127.0.0.1", 1883, "test/topic");

	kx_mq_set_connect_cb(mq, on_connect);
	kx_mq_set_subscribe_cb(mq, on_subscribe);
	kx_mq_set_message_cb(mq, on_message);
	kx_mq_set_log_cb(mq, on_logcb);

	kx_mq_loop_start(mq);

	kx_mq_free(mq);

    return 0;
}
#include <string.h>

#include "nng/nng.h"
#include "nng/protocol/mqtt/mqtt_parser.h"
#include "nng/mqtt/mqtt_client.h"

#include "mqtt_msg.h"
#include "nuts.h"

#define MQTT_MSG_DUMP 0

#if MQTT_MSG_DUMP
static void
#define DUMP_LENGTH 2048
print_mqtt_msg(nng_msg *msg)
{
	uint8_t print_buf[DUMP_LENGTH] = { 0 };
	nng_mqtt_msg_dump(msg, print_buf, DUMP_LENGTH, true);
	printf("\nmsg: \n%s\n", (char *) print_buf);
}
#else
static void
print_mqtt_msg(nng_msg *msg)
{
	NNI_ARG_UNUSED(msg);
	return;
}
#endif

void
test_alloc(void)
{
	nng_msg *msg;
	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));
	nng_msg_free(msg);
}

void
test_dup(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBSCRIBE);

	nng_mqtt_topic_qos topic_qos[] = {
		{ .qos     = 0,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/0",
		        .length     = strlen("/nanomq/mqtt/msg/0") } },
		{ .qos     = 1,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/1",
		        .length     = strlen("/nanomq/mqtt/msg/1") } }
	};
	nng_mqtt_msg_set_subscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic_qos));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	nng_msg *msg2;
	NUTS_PASS(nng_msg_dup(&msg2, msg));

	print_mqtt_msg(msg);
	print_mqtt_msg(msg2);

	NUTS_TRUE(memcmp(nng_msg_header(msg), nng_msg_header(msg2),
	              nng_msg_header_len(msg)) == 0);

	NUTS_TRUE(memcmp(nng_msg_body(msg), nng_msg_body(msg2),
	              nng_msg_len(msg)) == 0);

	nng_msg_free(msg2);
	nng_msg_free(msg);
}

void
test_dup_unsub(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_UNSUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_UNSUBSCRIBE);

	nng_mqtt_topic_qos topic_qos[] = {
		{ .qos     = 0,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/0",
		        .length     = strlen("/nanomq/mqtt/msg/0") } }
	};
	nng_mqtt_msg_set_subscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic_qos));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	nng_msg *msg2;
	NUTS_PASS(nng_msg_dup(&msg2, msg));

	print_mqtt_msg(msg);
	print_mqtt_msg(msg2);

	NUTS_TRUE(memcmp(nng_msg_header(msg), nng_msg_header(msg2),
	              nng_msg_header_len(msg)) == 0);

	NUTS_TRUE(memcmp(nng_msg_body(msg), nng_msg_body(msg2),
	              nng_msg_len(msg)) == 0);

	nng_msg_free(msg2);
	nng_msg_free(msg);
}

void
test_dup_suback(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBACK);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	nng_msg *msg2;
	NUTS_PASS(nng_msg_dup(&msg2, msg));

	print_mqtt_msg(msg);
	print_mqtt_msg(msg2);

	NUTS_TRUE(memcmp(nng_msg_header(msg), nng_msg_header(msg2),
	              nng_msg_header_len(msg)) == 0);

	NUTS_TRUE(memcmp(nng_msg_body(msg), nng_msg_body(msg2),
	              nng_msg_len(msg)) == 0);

	nng_msg_free(msg2);
	nng_msg_free(msg);
}

void
test_dup_publish(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBLISH);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBLISH);

	nng_mqtt_msg_set_publish_qos(msg, 0);
	nng_mqtt_msg_set_publish_topic(msg, "/nanomq/msg");
	nng_mqtt_msg_set_publish_payload(msg, (uint8_t *) "aaaaaaaa", 8);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	nng_mqtt_msg_set_publish_dup(msg, true);
	NUTS_TRUE(nng_mqtt_msg_get_publish_dup(msg));

	nng_msg *msg2;
	NUTS_PASS(nng_msg_dup(&msg2, msg));

	print_mqtt_msg(msg);
	print_mqtt_msg(msg2);

	NUTS_TRUE(memcmp(nng_msg_header(msg), nng_msg_header(msg2),
	              nng_msg_header_len(msg)) == 0);

	NUTS_TRUE(memcmp(nng_msg_body(msg), nng_msg_body(msg2),
	              nng_msg_len(msg)) == 0);

	nng_msg_free(msg);
	nng_msg_free(msg2);
}

void
test_encode_connect(void)
{
	nng_msg *msg;
	conn_param *cparam;
	char     client_id[] = "nanomq-mqtt";

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));
	// NUTS_PASS(conn_param_alloc(&cparam));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_CONNECT);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_CONNECT);

	char will_topic[] = "/nanomq/will_msg";
	nng_mqtt_msg_set_connect_will_topic(msg, will_topic);

	char will_msg[] = "Bye-bye";
	nng_mqtt_msg_set_connect_will_msg(msg, (uint8_t *)will_msg, strlen(will_msg));

	char user[]   = "nanomq";
	char passwd[] = "nanomq";

	nng_mqtt_msg_set_connect_client_id(msg, client_id);
	nng_mqtt_msg_set_connect_will_retain(msg, true);
	nng_mqtt_msg_set_connect_will_qos(msg, 2);
	nng_mqtt_msg_set_connect_user_name(msg, user);
	nng_mqtt_msg_set_connect_password(msg, passwd);
	nng_mqtt_msg_set_connect_clean_session(msg, true);
	nng_mqtt_msg_set_connect_keep_alive(msg, 60);

	NUTS_TRUE(strncmp(nng_mqtt_msg_get_connect_client_id(msg), client_id,
	              strlen(client_id)) == 0);
	NUTS_TRUE(nng_mqtt_msg_get_connect_will_retain(msg) == true);
	NUTS_TRUE(nng_mqtt_msg_get_connect_will_qos(msg) == 2);
	NUTS_TRUE(strncmp(nng_mqtt_msg_get_connect_user_name(msg), user, strlen(user)) == 0); 
	NUTS_TRUE(strncmp(nng_mqtt_msg_get_connect_password(msg), passwd, strlen(passwd)) == 0);
	NUTS_TRUE(nng_mqtt_msg_get_connect_clean_session(msg) == true);
	NUTS_TRUE(nng_mqtt_msg_get_connect_keep_alive(msg) == 60);

	cparam = nng_get_conn_param_from_msg(msg);

	nng_mqtt_msg_encode(msg);
	print_mqtt_msg(msg);

	nng_msg *decode_msg;
	nng_msg_dup(&decode_msg, msg);
	nng_msg_free(msg);

	nng_mqtt_msg_decode(decode_msg);
	print_mqtt_msg(decode_msg);

	if (nni_atomic_dec_nv(&cparam->refcnt) != 0) {
		return;
	}
	nng_free(cparam->pro_name.body, cparam->pro_name.len);
	nng_free(cparam->clientid.body, cparam->clientid.len);
	nng_free(cparam->will_topic.body, cparam->will_topic.len);
	nng_free(cparam->will_msg.body, cparam->will_msg.len);
	nng_free(cparam->username.body, cparam->username.len);
	nng_free(cparam->password.body, cparam->password.len);

	property_free(cparam->properties);
	property_free(cparam->will_properties);


	nng_free(cparam, sizeof(struct conn_param));

	nng_msg_free(decode_msg);
}

void
test_encode_connect_v5(void)
{
	nng_msg *msg;
	char     client_id[] = "nanomq-mqtt";

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_CONNECT);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_CONNECT);

	nng_mqtt_msg_set_connect_client_id(msg, client_id);

	NUTS_TRUE(strncmp(nng_mqtt_msg_get_connect_client_id(msg), client_id,
	              strlen(client_id)) == 0);

	char will_topic[] = "/nanomq/will_msg";
	nng_mqtt_msg_set_connect_will_topic(msg, will_topic);

	char will_msg[] = "Bye-bye";
	nng_mqtt_msg_set_connect_will_msg(msg, (uint8_t *)will_msg, strlen(will_msg));

	char user[]   = "nanomq";
	char passwd[] = "nanomq";

	nng_mqtt_msg_set_connect_user_name(msg, user);
	nng_mqtt_msg_set_connect_password(msg, passwd);
	nng_mqtt_msg_set_connect_clean_session(msg, true);
	nng_mqtt_msg_set_connect_keep_alive(msg, 60);

	property *p  = mqtt_property_alloc();
	property *p1 = mqtt_property_set_value_u32(MAXIMUM_PACKET_SIZE, 120);
	mqtt_property_append(p, p1);
	nng_mqtt_msg_set_connect_property(msg, p);

	nni_mqttv5_msg_encode(msg);
	print_mqtt_msg(msg);

	nng_msg *decode_msg;
	nng_msg_dup(&decode_msg, msg);
	nng_msg_free(msg);

	nni_mqttv5_msg_decode(decode_msg);
	print_mqtt_msg(decode_msg);

	nng_msg_free(decode_msg);
}

void
test_encode_connack(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_CONNACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_CONNACK);

	nng_mqtt_msg_set_connack_flags(msg, 1);
	nng_mqtt_msg_set_connack_return_code(msg, 0);

	NUTS_TRUE(nng_mqtt_msg_get_connack_flags(msg) == 1);
	NUTS_TRUE(nng_mqtt_msg_get_connack_return_code(msg) == 0);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_subscribe(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBSCRIBE);

	nng_mqtt_topic_qos topic_qos[] = {
		{ .qos     = 0,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/0",
		        .length     = strlen("/nanomq/mqtt/msg/0") + 1 } },
		{ .qos     = 1,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/1",
		        .length     = strlen("/nanomq/mqtt/msg/1") + 1 } }
	};

	nng_mqtt_msg_set_subscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic_qos));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_subscribe_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBSCRIBE);

	nng_mqtt_topic_qos topic_qos[] = {
		{ .qos     = 0,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/0",
		        .length     = strlen("/nanomq/mqtt/msg/0") + 1 } },
		{ .qos     = 1,
		    .topic = { .buf = (uint8_t *) "/nanomq/mqtt/msg/1",
		        .length     = strlen("/nanomq/mqtt/msg/1") + 1 } }
	};

	nng_mqtt_msg_set_subscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic_qos));

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_suback(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBACK);

	uint8_t ret_codes[] = { 0, 1, 2, 3 };

	nng_mqtt_msg_set_suback_return_codes(
	    msg, ret_codes, sizeof(ret_codes) / sizeof(uint8_t));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_suback_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_SUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_SUBACK);

	uint8_t ret_codes[] = { 0, 1, 2, 3 };

	nng_mqtt_msg_set_suback_return_codes(
	    msg, ret_codes, sizeof(ret_codes) / sizeof(uint8_t));

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_publish(void)
{
	nng_msg *msg;
	uint16_t pkt_id = 6;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBLISH);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBLISH);

	nng_mqtt_msg_set_publish_qos(msg, 2);
	nng_mqtt_msg_set_publish_retain(msg, true);

	NUTS_TRUE(nng_mqtt_msg_get_publish_qos(msg) == 2);
	NUTS_TRUE(nng_mqtt_msg_get_publish_retain(msg) == true);

	char *topic = "/nanomq/msg/18234";
	nng_mqtt_msg_set_publish_topic(msg, topic);
	nng_mqtt_msg_set_publish_topic_len(msg, strlen(topic));
	nng_mqtt_msg_set_publish_packet_id(msg, pkt_id);

	uint32_t topic_len = 0;
	NUTS_TRUE(strncmp(nng_mqtt_msg_get_publish_topic(msg, &topic_len),
	              topic, strlen(topic)) == 0);
	NUTS_TRUE(topic_len == strlen(topic));
	NUTS_TRUE(nng_mqtt_msg_get_publish_packet_id(msg) == pkt_id);

	char *payload = "hello";
	nng_mqtt_msg_set_publish_payload(
	    msg, (uint8_t *) payload, strlen(payload));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_encode_publish_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBLISH);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBLISH);

	nng_mqtt_msg_set_publish_qos(msg, 2);
	nng_mqtt_msg_set_publish_retain(msg, true);

	char *topic = "/nanomq/msg/18234";
	nng_mqtt_msg_set_publish_topic(msg, topic);

	char *payload = "hello";
	nng_mqtt_msg_set_publish_payload(
	    msg, (uint8_t *) payload, strlen(payload));

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_encode_puback(void)
{
	nng_msg *msg;
	uint16_t pkt_id = 2;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBACK);

	nng_mqtt_msg_set_puback_packet_id(msg, pkt_id);
	NUTS_TRUE(nng_mqtt_msg_get_puback_packet_id(msg) == pkt_id);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_puback_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBACK);

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubrec(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBREC);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBREC);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubrec_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBREC);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBREC);

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubrel(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBREL);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBREL);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubrel_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBREL);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBREL);

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubcomp(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBCOMP);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBCOMP);

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_pubcomp_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_PUBCOMP);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_PUBCOMP);

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_unsubscribe(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_UNSUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_UNSUBSCRIBE);

	nng_mqtt_topic topic_qos[] = {
		{ .buf      = (uint8_t *) "/nanomq/mqtt/1",
		    .length = strlen("/nanomq/mqtt/1") + 1 },
		{ .buf      = (uint8_t *) "/nanomq/mqtt/2",
		    .length = strlen("/nanomq/mqtt/2") + 1 },
	};

	nng_mqtt_msg_set_unsubscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic));
	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_unsubscribe_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_UNSUBSCRIBE);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_UNSUBSCRIBE);

	nng_mqtt_topic topic_qos[] = {
		{ .buf      = (uint8_t *) "/nanomq/mqtt/1",
		    .length = strlen("/nanomq/mqtt/1") + 1 },
		{ .buf      = (uint8_t *) "/nanomq/mqtt/2",
		    .length = strlen("/nanomq/mqtt/2") + 1 },
	};

	nng_mqtt_msg_set_unsubscribe_topics(
	    msg, topic_qos, sizeof(topic_qos) / sizeof(nng_mqtt_topic));
	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_unsuback(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_UNSUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_UNSUBACK);

	uint8_t ret_codes[] = { 0, 1, 2, 3 };

	nng_mqtt_msg_set_suback_return_codes(
	    msg, ret_codes, sizeof(ret_codes) / sizeof(uint8_t));

	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_unsuback_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_UNSUBACK);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_UNSUBACK);

	uint8_t ret_codes[] = { 0, 1, 2, 3 };

	nng_mqtt_msg_set_suback_return_codes(
	    msg, ret_codes, sizeof(ret_codes) / sizeof(uint8_t));

	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_encode_disconnect(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_DISCONNECT);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_DISCONNECT);
	nng_mqtt_msg_set_disconnect_reason_code(msg, 0);
	NUTS_PASS(nng_mqtt_msg_encode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_encode_disconnect_v5(void)
{
	nng_msg *msg;

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, 0));

	nng_mqtt_msg_set_packet_type(msg, NNG_MQTT_DISCONNECT);
	NUTS_TRUE(nng_mqtt_msg_get_packet_type(msg) == NNG_MQTT_DISCONNECT);
	NUTS_PASS(nng_mqttv5_msg_encode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_decode_connect(void)
{
	nng_msg *msg;
	uint8_t  connect[] = {

		0x10, 0x3f, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54, 0x04, 0xc6,
		0x00, 0x3c, 0x00, 0x0c, 0x54, 0x65, 0x73, 0x74, 0x2d, 0x43,
		0x6c, 0x69, 0x65, 0x6e, 0x74, 0x31, 0x00, 0x0a, 0x77, 0x69,
		0x6c, 0x6c, 0x5f, 0x74, 0x6f, 0x70, 0x69, 0x63, 0x00, 0x07,
		0x62, 0x79, 0x65, 0x2d, 0x62, 0x79, 0x65, 0x00, 0x05, 0x61,
		0x6c, 0x76, 0x69, 0x6e, 0x00, 0x09, 0x48, 0x48, 0x48, 0x31,
		0x32, 0x33, 0x34, 0x35, 0x36
	};

	size_t sz = sizeof(connect) / sizeof(uint8_t);

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, sz - 2));

	nng_msg_header_append(msg, connect, 2);

	memcpy(nng_msg_body(msg), connect + 2, sz - 2);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_decode_connack_v5(void)
{
	nng_msg *msg;
	uint8_t  connack[] = { 0x20, 0x13, 0x00, 0x00, 0x10, 0x27, 0x00, 0x10,
                0x00, 0x00, 0x25, 0x01, 0x2a, 0x01, 0x29, 0x01, 0x22, 0xff,
                0xff, 0x28, 0x01 };

	size_t sz = sizeof(connack) / sizeof(uint8_t);

	NUTS_PASS(nng_mqtt_msg_alloc(&msg, sz - 2));

	nng_msg_header_append(msg, connack, 2);

	memcpy(nng_msg_body(msg), connack + 2, sz - 2);

	NUTS_PASS(nni_mqttv5_msg_decode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_decode_subscribe(void)
{
	nng_msg *msg;

	uint8_t subscribe[] = { 0x82, 0x2c, 0x02, 0x10, 0x00, 0x12, 0x2f, 0x6e,
		0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74, 0x74,
		0x2f, 0x6d, 0x73, 0x67, 0x2f, 0x30, 0x00, 0x00, 0x12, 0x2f,
		0x6e, 0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74,
		0x74, 0x2f, 0x6d, 0x73, 0x67, 0x2f, 0x31, 0x01 };

	size_t sz = sizeof(subscribe) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, subscribe, 2);

	nng_msg_append(msg, subscribe + 2, sz - 2);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);

	// uint32_t            count;
	// nng_mqtt_topic_qos *tq =
	//     nng_mqtt_msg_get_subscribe_topics(msg, &count);

	nng_msg_free(msg);
}

void
test_decode_subscribe_v5(void)
{
	nng_msg *msg;

	uint8_t subscribe[] = { 0x82, 0x11, 0x93, 0x60, 0x00, 0x00, 0x0b, 0x74,
		0x65, 0x73, 0x74, 0x74, 0x6f, 0x70, 0x69, 0x63, 0x2f, 0x23,
		0x00 };

	size_t sz = sizeof(subscribe) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, subscribe, 2);

	nng_msg_append(msg, subscribe + 2, sz - 2);

	NUTS_PASS(nng_mqttv5_msg_decode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
	
}

void
test_decode_suback(void)
{
	nng_msg *msg;
	uint8_t  suback[] = { 0x90, 0x04, 0x02, 0x10, 0x00, 0x01 };
	size_t   sz       = sizeof(suback) / sizeof(uint8_t);

	nng_mqtt_msg_alloc(&msg, sz - 2);
	nng_msg_header_append(msg, suback, 2);
	memcpy(nng_msg_body(msg), suback + 2, sz - 2);
	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_decode_publish(void)
{
	nng_msg *msg;

	uint8_t publish[] = { 0x34, 0xba, 0x03, 0x00, 0x10, 0x2f, 0x6e, 0x61,
		0x6e, 0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74, 0x74, 0x2f,
		0x6d, 0x73, 0x67, 0x03, 0x6c, 0x7b, 0x22, 0x62, 0x72, 0x6f,
		0x6b, 0x65, 0x72, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x2f, 0x6e,
		0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x22, 0x2c, 0x22, 0x73, 0x64,
		0x6b, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x6d, 0x71, 0x74, 0x74,
		0x2d, 0x63, 0x6f, 0x64, 0x65, 0x63, 0x22, 0x2c, 0x22, 0x64,
		0x61, 0x74, 0x61, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x31, 0x39,
		0x33, 0x37, 0x38, 0x38, 0x39, 0x37, 0x36, 0x38, 0x39, 0x31,
		0x39, 0x33, 0x37, 0x39, 0x38, 0x37, 0x35, 0x38, 0x39, 0x37,
		0x33, 0x39, 0x31, 0x38, 0x37, 0x38, 0x39, 0x33, 0x37, 0x39,
		0x38, 0x35, 0x36, 0x37, 0x39, 0x38, 0x37, 0x31, 0x38, 0x39,
		0x37, 0x39, 0x34, 0x38, 0x37, 0x36, 0x39, 0x37, 0x39, 0x38,
		0x34, 0x37, 0x39, 0x38, 0x32, 0x37, 0x38, 0x39, 0x34, 0x37,
		0x38, 0x39, 0x36, 0x37, 0x34, 0x38, 0x33, 0x37, 0x32, 0x39,
		0x37, 0x39, 0x37, 0x34, 0x39, 0x37, 0x39, 0x32, 0x36, 0x37,
		0x39, 0x38, 0x33, 0x34, 0x32, 0x37, 0x39, 0x38, 0x34, 0x37,
		0x39, 0x38, 0x36, 0x37, 0x39, 0x38, 0x32, 0x37, 0x34, 0x39,
		0x38, 0x37, 0x36, 0x38, 0x39, 0x32, 0x37, 0x33, 0x34, 0x38,
		0x39, 0x37, 0x36, 0x32, 0x37, 0x39, 0x34, 0x37, 0x36, 0x37,
		0x32, 0x39, 0x38, 0x37, 0x41, 0x45, 0x46, 0x45, 0x46, 0x41,
		0x45, 0x46, 0x44, 0x43, 0x42, 0x46, 0x45, 0x41, 0x4b, 0x4a,
		0x53, 0x48, 0x46, 0x4b, 0x4a, 0x48, 0x53, 0x4a, 0x4b, 0x46,
		0x48, 0x4b, 0x4a, 0x53, 0x48, 0x4c, 0x4b, 0x4a, 0x4b, 0x55,
		0x49, 0x59, 0x49, 0x55, 0x45, 0x54, 0x49, 0x55, 0x51, 0x57,
		0x4f, 0x49, 0x51, 0x4f, 0x3c, 0x4d, 0x5a, 0x4e, 0x3c, 0x4d,
		0x42, 0x4a, 0x48, 0x47, 0x48, 0x4a, 0x46, 0x48, 0x47, 0x4c,
		0x4b, 0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41, 0x51, 0x57,
		0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x5a, 0x58,
		0x43, 0x56, 0x42, 0x4e, 0x4d, 0x39, 0x38, 0x38, 0x32, 0x34,
		0x37, 0x35, 0x39, 0x32, 0x38, 0x37, 0x38, 0x39, 0x37, 0x35,
		0x34, 0x39, 0x38, 0x32, 0x37, 0x39, 0x38, 0x35, 0x37, 0x61,
		0x64, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x46, 0x47,
		0x48, 0x4a, 0x46, 0x47, 0x48, 0x4a, 0x47, 0x48, 0x4a, 0x47,
		0x48, 0x4a, 0x46, 0x47, 0x48, 0x47, 0x48, 0x4a, 0x47, 0x48,
		0x47, 0x48, 0x4a, 0x44, 0x46, 0x31, 0x39, 0x33, 0x37, 0x38,
		0x38, 0x39, 0x37, 0x36, 0x38, 0x39, 0x31, 0x39, 0x33, 0x37,
		0x39, 0x38, 0x37, 0x35, 0x38, 0x39, 0x37, 0x33, 0x39, 0x31,
		0x38, 0x37, 0x38, 0x39, 0x33, 0x37, 0x39, 0x38, 0x35, 0x36,
		0x37, 0x39, 0x38, 0x37, 0x31, 0x38, 0x39, 0x37, 0x39, 0x34,
		0x38, 0x37, 0x36, 0x39, 0x37, 0x39, 0x38, 0x34, 0x37, 0x39,
		0x38, 0x32, 0x37, 0x38, 0x39, 0x64, 0x6a, 0x61, 0x6b, 0x68,
		0x6b, 0x6a, 0x68, 0x65, 0x71, 0x69, 0x75, 0x79, 0x69, 0x65,
		0x75, 0x79, 0x69, 0x75, 0x74, 0x79, 0x69, 0x75, 0x71, 0x79,
		0x69, 0x75, 0x79, 0x69, 0x75, 0x22, 0x7d };

	size_t sz = sizeof(publish) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, sz - 3);

	nng_msg_header_append(msg, publish, 3);
	memcpy(nng_msg_body(msg), publish + 3, sz - 3);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);

	nng_msg_free(msg);
}

void
test_decode_puback(void)
{
	nng_msg *msg;

	uint8_t puback[] = { 0x40, 0x02, 0x01, 0x20 };
	size_t  sz       = sizeof(puback) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, puback, sz - 2);

	nng_msg_append(msg, puback + 2, sz - 2);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_decode_puback_v5(void)
{
	nng_msg *msg;

	uint8_t puback[] = { 0x40, 0x02, 0x01, 0x20 };
	size_t  sz       = sizeof(puback) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, puback, sz - 2);

	nng_msg_append(msg, puback + 2, sz - 2);

	NUTS_PASS(nng_mqttv5_msg_decode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_decode_unsubscribe(void)
{
	nng_msg *msg;

	uint8_t unsubscribe[] = { 0xa2, 0x24, 0x00, 0x00, 0x00, 0x0f, 0x2f,
		0x6e, 0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74,
		0x74, 0x2f, 0x31, 0x00, 0x00, 0x0f, 0x2f, 0x6e, 0x61, 0x6e,
		0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74, 0x74, 0x2f, 0x32,
		0x00 };

	size_t sz = sizeof(unsubscribe) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, unsubscribe, 2);
	nng_msg_append(msg, unsubscribe + 2, sz - 2);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);

	// uint32_t        count;
	// nng_mqtt_topic *topics =
	//     nng_mqtt_msg_get_unsubscribe_topics(msg, &count);

	nng_msg_free(msg);
}

void
test_decode_unsubscribe_v5(void)
{
	nng_msg *msg;

	uint8_t unsubscribe[] = { 0xa2, 0x10, 0x93, 0x73, 0x00, 0x00, 0x0b,
		0x74, 0x65, 0x73, 0x74, 0x74, 0x6f, 0x70, 0x69, 0x63, 0x2f,
		0x23 };

	size_t sz = sizeof(unsubscribe) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, unsubscribe, 2);
	nng_msg_append(msg, unsubscribe + 2, sz - 2);

	NUTS_PASS(nng_mqttv5_msg_decode(msg));

	print_mqtt_msg(msg);

	// uint32_t        count;
	// nng_mqtt_topic *topics =
	//     nng_mqtt_msg_get_unsubscribe_topics(msg, &count);

	nng_msg_free(msg);
}

void
test_decode_disconnect(void)
{
	nng_msg *msg;
	uint8_t  disconnect[] = { 0xe0, 0x00 };

	size_t sz = sizeof(disconnect) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, disconnect, sz);

	NUTS_PASS(nng_mqtt_msg_decode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

void
test_decode_disconnect_v5(void)
{
	nng_msg *msg;
	uint8_t  disconnect[] = { 0xe0, 0x02, 0x00, 0x00 };

	size_t sz = sizeof(disconnect) / sizeof(uint8_t);
	nng_mqtt_msg_alloc(&msg, 0);

	nng_msg_header_append(msg, disconnect, 2);
	nng_msg_append(msg, disconnect + 2, sz - 2);

	NUTS_PASS(nng_mqttv5_msg_decode(msg));

	print_mqtt_msg(msg);
	nng_msg_free(msg);
}

static nni_msg *
create_msg(uint8_t *packet, size_t sz, size_t header_sz)
{
	nni_msg *msg;
	nng_mqtt_msg_alloc(&msg, sz - header_sz);
	nng_msg_header_append(msg, packet, header_sz);

	memcpy(nng_msg_body(msg), packet + header_sz, sz - header_sz);
	return msg;
}

void
test_packet_validate(void)
{
	uint8_t invalid_packet1[] = { 0x10, 0x41, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x04,
		0x5c, 0x13, 0xc2, 0xb1, 0x19, 0x00, 0x0b, 0x66, 0x65, 0x72,
		0x72, 0x4c, 0xd9, 0x45, 0x41, 0x5a, 0x66, 0xf8, 0x35, 0x32,
		0x00, 0x18, 0xe3, 0x61, 0x77, 0x69, 0x34, 0x43, 0x30, 0x48,
		0x50, 0x60, 0x62, 0x76, 0x70, 0x66, 0x31, 0x46, 0x40, 0x6f,
		0x65, 0x55, 0x00, 0xf0, 0x73, 0x57, 0x7a, 0x33, 0x48, 0x6d,
		0x76, 0x73, 0x45, 0x66, 0x4d, 0x44, 0x6e };

	uint8_t invalid_packet2[] = { 0x10, 0x49, 0x00, 0x04, 0x4d, 0x51, 0x54, 0x54,
		0x05, 0x06, 0x00, 0x3c, 0x03, 0x22, 0x00, 0x0a, 0x00, 0x17,
		0x6d, 0x71, 0x74, 0x74, 0x6f, 0x6f, 0x6c, 0x73, 0x2d, 0x66,
		0x33, 0x35, 0x62, 0x37, 0x39, 0x36, 0x32, 0x39, 0x35, 0x34,
		0x38, 0x31, 0x31, 0x00, 0x00, 0x0e, 0x2f, 0x6d, 0x79, 0x2f,
		0x77, 0x69, 0x6c, 0x6c, 0x2f, 0x74, 0x6f, 0x70, 0x69, 0x63,
		0x00, 0x0f, 0x6d, 0x92, 0xd7, 0x76, 0x96, 0xc6, 0xc2, 0xd6,
		0xd6, 0x57, 0x37, 0x36, 0x16, 0x76, 0x56, 0x20, 0x20, 0x00,
		0x15, 0x00, 0x20, 0x00, 0x10 };

	uint8_t valid_packet3[] = { 0x20, 0x13, 0x00, 0x00, 0x10, 0x27, 0x00,
		0x10, 0x00, 0x00, 0x25, 0x01, 0x2a, 0x01, 0x29, 0x01, 0x22,
		0xff, 0xff, 0x28, 0x01 };

	uint8_t valid_packet4[] = { 0x34, 0xba, 0x03, 0x00, 0x10, 0x2f, 0x6e,
		0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x2f, 0x6d, 0x71, 0x74, 0x74,
		0x2f, 0x6d, 0x73, 0x67, 0x03, 0x6c, 0x7b, 0x22, 0x62, 0x72,
		0x6f, 0x6b, 0x65, 0x72, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x2f,
		0x6e, 0x61, 0x6e, 0x6f, 0x6d, 0x71, 0x22, 0x2c, 0x22, 0x73,
		0x64, 0x6b, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x6d, 0x71, 0x74,
		0x74, 0x2d, 0x63, 0x6f, 0x64, 0x65, 0x63, 0x22, 0x2c, 0x22,
		0x64, 0x61, 0x74, 0x61, 0x22, 0x20, 0x3a, 0x20, 0x22, 0x31,
		0x39, 0x33, 0x37, 0x38, 0x38, 0x39, 0x37, 0x36, 0x38, 0x39,
		0x31, 0x39, 0x33, 0x37, 0x39, 0x38, 0x37, 0x35, 0x38, 0x39,
		0x37, 0x33, 0x39, 0x31, 0x38, 0x37, 0x38, 0x39, 0x33, 0x37,
		0x39, 0x38, 0x35, 0x36, 0x37, 0x39, 0x38, 0x37, 0x31, 0x38,
		0x39, 0x37, 0x39, 0x34, 0x38, 0x37, 0x36, 0x39, 0x37, 0x39,
		0x38, 0x34, 0x37, 0x39, 0x38, 0x32, 0x37, 0x38, 0x39, 0x34,
		0x37, 0x38, 0x39, 0x36, 0x37, 0x34, 0x38, 0x33, 0x37, 0x32,
		0x39, 0x37, 0x39, 0x37, 0x34, 0x39, 0x37, 0x39, 0x32, 0x36,
		0x37, 0x39, 0x38, 0x33, 0x34, 0x32, 0x37, 0x39, 0x38, 0x34,
		0x37, 0x39, 0x38, 0x36, 0x37, 0x39, 0x38, 0x32, 0x37, 0x34,
		0x39, 0x38, 0x37, 0x36, 0x38, 0x39, 0x32, 0x37, 0x33, 0x34,
		0x38, 0x39, 0x37, 0x36, 0x32, 0x37, 0x39, 0x34, 0x37, 0x36,
		0x37, 0x32, 0x39, 0x38, 0x37, 0x41, 0x45, 0x46, 0x45, 0x46,
		0x41, 0x45, 0x46, 0x44, 0x43, 0x42, 0x46, 0x45, 0x41, 0x4b,
		0x4a, 0x53, 0x48, 0x46, 0x4b, 0x4a, 0x48, 0x53, 0x4a, 0x4b,
		0x46, 0x48, 0x4b, 0x4a, 0x53, 0x48, 0x4c, 0x4b, 0x4a, 0x4b,
		0x55, 0x49, 0x59, 0x49, 0x55, 0x45, 0x54, 0x49, 0x55, 0x51,
		0x57, 0x4f, 0x49, 0x51, 0x4f, 0x3c, 0x4d, 0x5a, 0x4e, 0x3c,
		0x4d, 0x42, 0x4a, 0x48, 0x47, 0x48, 0x4a, 0x46, 0x48, 0x47,
		0x4c, 0x4b, 0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41, 0x51,
		0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x5a,
		0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x39, 0x38, 0x38, 0x32,
		0x34, 0x37, 0x35, 0x39, 0x32, 0x38, 0x37, 0x38, 0x39, 0x37,
		0x35, 0x34, 0x39, 0x38, 0x32, 0x37, 0x39, 0x38, 0x35, 0x37,
		0x61, 0x64, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x46,
		0x47, 0x48, 0x4a, 0x46, 0x47, 0x48, 0x4a, 0x47, 0x48, 0x4a,
		0x47, 0x48, 0x4a, 0x46, 0x47, 0x48, 0x47, 0x48, 0x4a, 0x47,
		0x48, 0x47, 0x48, 0x4a, 0x44, 0x46, 0x31, 0x39, 0x33, 0x37,
		0x38, 0x38, 0x39, 0x37, 0x36, 0x38, 0x39, 0x31, 0x39, 0x33,
		0x37, 0x39, 0x38, 0x37, 0x35, 0x38, 0x39, 0x37, 0x33, 0x39,
		0x31, 0x38, 0x37, 0x38, 0x39, 0x33, 0x37, 0x39, 0x38, 0x35,
		0x36, 0x37, 0x39, 0x38, 0x37, 0x31, 0x38, 0x39, 0x37, 0x39,
		0x34, 0x38, 0x37, 0x36, 0x39, 0x37, 0x39, 0x38, 0x34, 0x37,
		0x39, 0x38, 0x32, 0x37, 0x38, 0x39, 0x64, 0x6a, 0x61, 0x6b,
		0x68, 0x6b, 0x6a, 0x68, 0x65, 0x71, 0x69, 0x75, 0x79, 0x69,
		0x65, 0x75, 0x79, 0x69, 0x75, 0x74, 0x79, 0x69, 0x75, 0x71,
		0x79, 0x69, 0x75, 0x79, 0x69, 0x75, 0x22, 0x7d };

	TEST_CHECK(nni_mqtt_msg_packet_validate(invalid_packet1,
	               sizeof(invalid_packet1) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v311) != MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(invalid_packet1,
	               sizeof(invalid_packet1) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v5) != MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(invalid_packet2,
	               sizeof(invalid_packet2) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v311) != MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(invalid_packet2,
	               sizeof(invalid_packet2) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v5) == MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(valid_packet3,
	               sizeof(valid_packet3) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v311) == MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(valid_packet3,
	               sizeof(valid_packet3) / sizeof(uint8_t), 2,
	               MQTT_PROTOCOL_VERSION_v5) == MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(valid_packet4,
	               sizeof(valid_packet4) / sizeof(uint8_t), 3,
	               MQTT_PROTOCOL_VERSION_v311) == MQTT_SUCCESS);

	TEST_CHECK(nni_mqtt_msg_packet_validate(valid_packet4,
	               sizeof(valid_packet4) / sizeof(uint8_t), 3,
	               MQTT_PROTOCOL_VERSION_v5) == MQTT_SUCCESS);

	nni_msg *msg1 = create_msg(
	    invalid_packet1, sizeof(invalid_packet1) / sizeof(uint8_t), 2);
	nni_msg *msg2 = create_msg(
	    invalid_packet2, sizeof(invalid_packet2) / sizeof(uint8_t), 2);
	nni_msg *msg3 =
	    create_msg(valid_packet3, sizeof(valid_packet3) / sizeof(uint8_t), 2);
	nni_msg *msg4 =
	    create_msg(valid_packet4, sizeof(valid_packet4) / sizeof(uint8_t), 3);

	TEST_CHECK(nng_mqtt_msg_validate(msg1, MQTT_PROTOCOL_VERSION_v311) !=
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg1, MQTT_PROTOCOL_VERSION_v5) !=
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg2, MQTT_PROTOCOL_VERSION_v311) !=
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg2, MQTT_PROTOCOL_VERSION_v5) ==
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg3, MQTT_PROTOCOL_VERSION_v311) ==
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg3, MQTT_PROTOCOL_VERSION_v5) ==
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg4, MQTT_PROTOCOL_VERSION_v311) ==
	    MQTT_SUCCESS);

	TEST_CHECK(nng_mqtt_msg_validate(msg4, MQTT_PROTOCOL_VERSION_v5) ==
	    MQTT_SUCCESS);

	nng_msg_free(msg1);
	nng_msg_free(msg2);
	nng_msg_free(msg3);
	nng_msg_free(msg4);
}

void
test_byte_number_for_var_len(void)
{
	NUTS_TRUE(byte_number_for_variable_length(127) == 1);
	NUTS_TRUE(byte_number_for_variable_length(16383) == 2);
	NUTS_TRUE(byte_number_for_variable_length(2097151) == 3);
	NUTS_TRUE(byte_number_for_variable_length(268435455) == 4);
	NUTS_TRUE(byte_number_for_variable_length(268435457) == 5);
}

void
test_mqtt_msg_dump(void)
{
	nng_msg *connmsg;
	nng_mqtt_msg_alloc(&connmsg, 0);
	nng_mqtt_msg_set_packet_type(connmsg, NNG_MQTT_CONNECT);
	nng_mqtt_msg_set_connect_proto_version(connmsg, 4);
	nng_mqtt_msg_set_connect_keep_alive(connmsg, 60);
	nng_mqtt_msg_set_connect_user_name(connmsg, "nng_mqtt_client");
	nng_mqtt_msg_set_connect_password(connmsg, "secrets");
	nng_mqtt_msg_set_connect_will_msg(
	    connmsg, (uint8_t *) "bye-bye", strlen("bye-bye"));
	nng_mqtt_msg_set_connect_will_topic(connmsg, "will_topic");
	nng_mqtt_msg_set_connect_clean_session(connmsg, true);

	uint8_t buff[1024] = { 0 };

	nng_mqtt_msg_dump(connmsg, buff, sizeof(buff), true);
}

void
test_kv(void)
{
	mqtt_kv *kv1   = NULL;
	mqtt_kv *kv2   = NULL;
	char    *key   = "key-test";
	char    *value = "value-test";

	NUTS_TRUE((kv1 = NNI_ALLOC_STRUCT(kv1)) != NULL);
	NUTS_TRUE((kv2 = NNI_ALLOC_STRUCT(kv2)) != NULL);

	NUTS_PASS(mqtt_kv_create(kv1, key, strlen(key), value, strlen(value)));
	NUTS_PASS(mqtt_kv_dup(kv2, kv1));
	mqtt_kv_free(kv1);
	mqtt_kv_free(kv2);

	NNI_FREE_STRUCT(kv1);
	NNI_FREE_STRUCT(kv2);
}

void
test_write_read(void)
{
	int       length     = 50;
	uint8_t   val8       = 8;
	uint16_t  val16      = 16;
	uint32_t  val32      = 32;
	uint64_t  val64      = 64;
	char     *bytes      = "bytes";
	char     *str        = "mqtt_str";
	mqtt_buf *mqtt_buf_1 = NULL;
	mqtt_buf_1           = NNI_ALLOC_STRUCT(mqtt_buf_1);
	NUTS_PASS(mqtt_buf_create(mqtt_buf_1, (uint *) str, strlen(str)));
	uint8_t       *data = calloc(length, sizeof(char));
	struct pos_buf buf;
	buf.curpos = &data[0];
	buf.endpos = &data[length];

	NUTS_PASS(write_byte(val8, &buf));
	NUTS_PASS(write_uint16(val16, &buf));
	NUTS_PASS(write_uint32(val32, &buf));
	NUTS_PASS(write_uint64(val64, &buf));
	NUTS_PASS(write_bytes((uint *) bytes, sizeof(bytes), &buf));
	NUTS_PASS(write_byte_string(mqtt_buf_1, &buf));

	uint8_t   _val8      = 0;
	uint16_t  _val16     = 0;
	uint32_t  _val32     = 0;
	uint64_t  _val64     = 0;
	char     *strVal     = NULL;
	mqtt_buf *mqtt_buf_2 = NULL;
	mqtt_buf_2           = NNI_ALLOC_STRUCT(mqtt_buf_2);
	buf.curpos           = &data[0];
	NUTS_PASS(read_byte(&buf, &_val8));
	NUTS_TRUE(_val8 == val8);
	NUTS_PASS(read_uint16(&buf, &_val16));
	NUTS_TRUE(_val16 == val16);
	NUTS_PASS(read_uint32(&buf, &_val32));
	NUTS_TRUE(_val32 == val32);
	NUTS_PASS(read_uint64(&buf, &_val64));
	NUTS_TRUE(_val64 == val64);
	NUTS_PASS(read_bytes(&buf, &strVal, sizeof(bytes)));
	NUTS_TRUE(strncmp(strVal, bytes, sizeof(bytes)) == 0);
	NUTS_PASS(read_str_data(&buf, mqtt_buf_2));
	NUTS_TRUE(mqtt_buf_2->length == mqtt_buf_1->length);
	NUTS_TRUE(strncmp((char *) mqtt_buf_2->buf, (char *) mqtt_buf_2->buf,
	              mqtt_buf_2->length) == 0);

	NNI_FREE_STRUCT(mqtt_buf_1);
	NNI_FREE_STRUCT(mqtt_buf_2);
	free(data);
}

void
test_msg_create_destroy(void)
{
	mqtt_msg *msg = NULL;
	NUTS_TRUE((msg = mqtt_msg_create(NNG_MQTT_CONNECT)) != NULL);
	NUTS_PASS(mqtt_msg_destroy(msg));
}

TEST_LIST = {
	// TODO: there is still some encode & decode functions should be tested.
	{ "alloc message", test_alloc },
	{ "dup message", test_dup },
	{ "dup unsub message", test_dup_unsub },
	{ "dup suback message", test_dup_suback },
	{ "dup publish message", test_dup_publish },
	{ "encode connect", test_encode_connect },
	{ "encode connect v5", test_encode_connect_v5 },
	{ "encode conack", test_encode_connack },
	{ "decode connack v5", test_decode_connack_v5 },
	{ "encode disconnect v5", test_encode_disconnect_v5 },
	{ "encode subscribe", test_encode_subscribe },
	{ "encode subscribe v5", test_encode_subscribe_v5 },
	{ "encode suback", test_encode_suback },
	{ "encode suback v5", test_encode_suback_v5 },
	{ "encode publish", test_encode_publish },
	{ "encode publish v5", test_encode_publish_v5 },
	{ "encode puback", test_encode_puback },
	{ "encode puback v5", test_encode_puback_v5 },
	{ "encode pubrec", test_encode_pubrec },
	{ "encode pubrec v5", test_encode_pubrec_v5 },
	{ "encode pubrel", test_encode_pubrel },
	{ "encode pubrel v5", test_encode_pubrel_v5 },
	{ "encode pubcomp", test_encode_pubcomp },
	{ "encode pubcomp v5", test_encode_pubcomp_v5 },
	{ "encode unsubscribe", test_encode_unsubscribe },
	{ "encode unsubscribe v5", test_encode_unsubscribe_v5 },
	{ "encode unsuback", test_encode_unsuback },
	{ "encode unsuback v5", test_encode_unsuback_v5 },
	{ "encode disconnect", test_encode_disconnect },
	{ "decode connect", test_decode_connect },
	{ "decode subscribe", test_decode_subscribe },
	{ "decode subscribe v5", test_decode_subscribe_v5 },
	{ "decode suback", test_decode_suback },
	{ "decode publish", test_decode_publish },
	{ "decode puback", test_decode_puback },
	{ "decode puback v5", test_decode_puback_v5 },
	{ "decode unsubscribe", test_decode_unsubscribe },
	{ "decode unsubscribe v5", test_decode_unsubscribe_v5 },
	{ "decode disconnect", test_decode_disconnect },
	{ "decode disconnect v5", test_decode_disconnect_v5 },
	{ "validate packet", test_packet_validate },
	{ "byte_number_for_var_len", test_byte_number_for_var_len },
	{ "test mqtt msg dump", test_mqtt_msg_dump },
	{ "test kv", test_kv },
	{ "test write & read", test_write_read },
	{ "test create & destroy", test_msg_create_destroy },
	{ NULL, NULL },
};
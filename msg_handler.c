//
// Created by Хумай Байрамова on 04.12.2022.
//

#include "msg_handler.h"

#include <stdlib.h>
#include <string.h>

timestamp_t lamport_time = 0;

timestamp_t get_lamport_time() {
    return lamport_time;
}

void inc_lamport_time() {
    lamport_time = lamport_time + 1;
}

void set_lamport_time(timestamp_t t) {
    lamport_time = t;
}

Message *prepare_msg(void *payload, uint16_t payload_len, int16_t type) {

    inc_lamport_time();

    Message *msg = malloc(sizeof(MessageHeader) + payload_len);
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = type;
    msg->s_header.s_local_time = get_lamport_time();
    msg->s_header.s_payload_len = payload_len;

    memcpy(msg->s_payload, payload, msg->s_header.s_payload_len);

    return msg;
}

Message *remessage(Message* msg) {
    inc_lamport_time();
    msg->s_header.s_local_time = get_lamport_time();
    return msg;
}

void handle_msg(Message *msg) {
    timestamp_t curr_time = get_lamport_time();
    if (msg->s_header.s_local_time > curr_time) set_lamport_time(msg->s_header.s_local_time);
    inc_lamport_time();
}

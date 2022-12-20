//
// Created by Хумай Байрамова on 04.12.2022.
//
#include "ipc.h"
#include <unistd.h>

#ifndef DISTRIBUTED_SYSTEMS_2_MSG_HANDLER_H
#define DISTRIBUTED_SYSTEMS_2_MSG_HANDLER_H

Message* prepare_msg(void *payload, uint16_t payload_len, int16_t type);

void handle_msg(Message* msg);

Message *remessage(Message* msg);

timestamp_t get_lamport_time();

void inc_lamport_time();

void set_lamport_time(timestamp_t t);

#endif //DISTRIBUTED_SYSTEMS_2_MSG_HANDLER_H

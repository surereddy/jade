/*
 * zmq_handler.h
 *
 *  Created on: Dec 20, 2017
 *      Author: pchero
 */

#ifndef SRC_ZMQ_HANDLER_H_
#define SRC_ZMQ_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

bool init_zmq_handler(void);
void term_zmq_handler(void);

bool publish_message(const char* pub_target, json_t* j_data);
void* get_zmq_context(void);
const char* get_zmq_pub_addr(void);

int s_send(void* socket, const char* data);
char* s_recv(void* socket);

#endif /* SRC_ZMQ_HANDLER_H_ */

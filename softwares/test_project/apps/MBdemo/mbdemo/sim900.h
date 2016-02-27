/*
 * sim900.h
 *
 *  Created on: Feb 27, 2016
 *      Author: Nguyen Dinh Trung Truc
 */

#ifndef SIM900_H_
#define SIM900_H_

#include "ring_buffer.h"
#include "MB1_System.h"

extern rb_handle_t rxBuffer;

void sim900_Init(void);

void sim900_makeCall(uint8_t *number);

void sim900_sendMsg(uint8_t *number, uint8_t *msg);

#endif /* SIM900_H_ */

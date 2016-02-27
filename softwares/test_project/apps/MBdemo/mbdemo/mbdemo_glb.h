/*
 * mbdemo_glb.h
 *
 *  Created on: Feb 24, 2016
 *      Author: nvhien1992
 */

#ifndef MBDEMO_GLB_H_
#define MBDEMO_GLB_H_

extern "C" {
#include "thread.h"
}

#define BTN0_ID 0x01
#define BTN1_ID 0x10
#define SHELL_ID 0x12
#define UART_ID 0x22

#define SMS_ID 0x23
#define VC_ID 0x24
#define VC_END_ID 0x25
#define VC_END_BS_ID 0x26
#define VC_END_NO_ID 0x27

namespace mbdemo_ns {
const uint8_t max_threads = 3;
extern kernel_pid_t thread_pid[max_threads];
}

extern char phone_number[15];

#endif /* MBDEMO_GLB_H_ */

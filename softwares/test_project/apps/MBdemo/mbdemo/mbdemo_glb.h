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
#define LCD_ID 0x11
#define SIM900_ID 0x12

namespace mbdemo_ns {
const uint8_t max_threads = 2;
extern kernel_pid_t thread_pid[max_threads];
}

extern char phone_number[12];

#endif /* MBDEMO_GLB_H_ */

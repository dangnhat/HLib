/*
 * mbdemo.h
 *
 *  Created on: Feb 24, 2016
 *      Author: nvhien1992
 */

#ifndef MBDEMO_H_
#define MBDEMO_H_

#include "mbdemo_glb.h"
#include "ha_shell.h"

void mbdemo_init(void);
void* sim900a_handler(void* arg);
void* btn_lcd_handler(void* arg);

#endif /* MBDEMO_H_ */

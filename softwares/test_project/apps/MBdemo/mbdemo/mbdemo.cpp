/*
 * mbdemo.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: nvhien1992
 */
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "msg.h"
#include "thread.h"
}
#include "mbdemo.h"
#include "MB1_System.h"
#include "button_driver.h"
#include "cLCD.h"

using namespace gpio_ns;
using namespace btn_ns;

const ISRMgr_ns::ISR_t tim_isr_type = ISRMgr_ns::ISRMgr_TIM6;

kernel_pid_t mbdemo_ns::thread_pid[mbdemo_ns::max_threads];

static const uint8_t queue_handler_size = 16;

gpio_params_t btn0_gpio = {
        port_B,
        0,
        in_floating,
        speed_2MHz
};

gpio_params_t btn1_gpio = {
        port_B,
        1,
        in_floating,
        speed_2MHz
};

gpio_params_t lcdRS_gpio = {
        port_B,//A,
        11,//2,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdEN_gpio = {
        port_B,//A,
        10,//1,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdBL_gpio = {
        port_C,
        6,//13,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdD4_gpio = {
        port_B,
        12,//6,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdD5_gpio = {
        port_B,
        13,//7,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdD6_gpio = {
        port_B,
        14,//8,
        out_push_pull,
        speed_10MHz
};

gpio_params_t lcdD7_gpio = {
        port_B,
        15,//9,
        out_push_pull,
        speed_10MHz
};

void mbdemo_init(void) {
    //MBoard init
    MB1_system_init();

    //GPIO init for LCD
    lcdRS.gpio_init(&lcdRS_gpio);
    lcdEN.gpio_init(&lcdEN_gpio);
    lcdBL.gpio_init(&lcdBL_gpio);
    lcdD4.gpio_init(&lcdD4_gpio);
    lcdD5.gpio_init(&lcdD5_gpio);
    lcdD6.gpio_init(&lcdD6_gpio);
    lcdD7.gpio_init(&lcdD7_gpio);

    //LCD init
    cLCD_Init();
    cLCD_Clear();
}

void* sim900a_handler(void* arg)
{
    printf("==== sim900a thread ====\n");
    msg_t msg_q[queue_handler_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_handler_size);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == SIM900_ID) {
            //make a voice call
            printf("%s\n", (char*) msg.content.ptr);
            cLCD_Clear();
            cLCD_Printf("%s", (char*) msg.content.ptr);
        }
    }

    return NULL;
}

void* btn_lcd_handler(void* arg)
{
    printf("==== btn-lcd thread ====\n");

    button btn0, btn1;
    msg_t msg_q[queue_handler_size];

    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_handler_size);

    btn0.button_configure(&btn0_gpio);
    btn0.set_btn_id(BTN0_ID);
    btn1.button_configure(&btn1_gpio);
    btn1.set_btn_id(BTN1_ID);
    MB1_ISRs.subISR_assign(tim_isr_type, btn_callback_timer_isr);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        switch(msg.type) {
        case BTN0_ID: //up-button
            printf("btn0: %d\n", (uint8_t)msg.content.value);
            break;
        case BTN1_ID: //down-button
            printf("btn1: %d\n", (uint8_t)msg.content.value);
            break;
        case LCD_ID: //receive msg from sim900a thread.

            break;
        default:
            break;
        }
    }

    return NULL;
}

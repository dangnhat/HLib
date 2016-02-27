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
#include "sim900.h"

using namespace gpio_ns;
using namespace btn_ns;

const ISRMgr_ns::ISR_t tim_isr_type = ISRMgr_ns::ISRMgr_TIM6;

kernel_pid_t mbdemo_ns::thread_pid[mbdemo_ns::max_threads];

static const uint8_t queue_handler_size = 128;

gpio_params_t btn0_gpio = { port_B, 9, in_floating, speed_2MHz };

gpio_params_t btn1_gpio = { port_B, 8, in_floating, speed_2MHz };

gpio_params_t lcdRS_gpio = { port_B, //A,
        11, //2,
        out_push_pull, speed_10MHz };

gpio_params_t lcdEN_gpio = { port_B, //A,
        10, //1,
        out_push_pull, speed_10MHz };

gpio_params_t lcdBL_gpio = { port_C, 6, //13,
        out_push_pull, speed_10MHz };

gpio_params_t lcdD4_gpio = { port_B, 12, //6,
        out_push_pull, speed_10MHz };

gpio_params_t lcdD5_gpio = { port_B, 13, //7,
        out_push_pull, speed_10MHz };

gpio_params_t lcdD6_gpio = { port_B, 14, //8,
        out_push_pull, speed_10MHz };

gpio_params_t lcdD7_gpio = { port_B, 15, //9,
        out_push_pull, speed_10MHz };

static const uint32_t baudrate = 115200;
static const ISRMgr_ns::ISR_t MB1_int_type = ISRMgr_ns::ISRMgr_USART2;

static void usart2_irq(void);

void mbdemo_init(void)
{
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

    MB1_USART2.it_enable(0, 1);
    MB1_USART2.it_config(USART_IT_RXNE, ENABLE);
    MB1_ISRs.subISR_assign(MB1_int_type, usart2_irq);

    MB1_USART2.Restart(baudrate);

    sim900_Init();
    cLCD_Clear();
    cLCD_Printf("  WELCOME TO\n     HCMUT");
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
        if (msg.type == SHELL_ID) {
            sim900_makeCall((uint8_t *) msg.content.ptr);
            cLCD_Clear();
            cLCD_Printf("    CALLING\n%s ...", msg.content.ptr);
        } else if (msg.type == VC_ID) {
            cLCD_Clear();
            cLCD_Printf(" INCOMING CALL\n%s ...", msg.content.ptr);
        } else if (msg.type == VC_END_ID) {
            cLCD_Clear();
            cLCD_Printf("  MISSED CALL");
        } else if (msg.type == VC_END_BS_ID) {
            cLCD_Clear();
            cLCD_Printf("      BUSY");
        } else if (msg.type == VC_END_NO_ID) {
            cLCD_Clear();
            cLCD_Printf("  NO ANSWER");
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
        switch (msg.type) {
        case BTN0_ID: //D
            //printf("btn_up: %d\n", (uint8_t) msg.content.value);
            cLCD_Clear();
            MB1_USART2.Print("ATH\r");
            break;
        case BTN1_ID: //U
            //printf("btn1: %d\n", (uint8_t) msg.content.value);
            cLCD_Clear();
            MB1_USART2.Print("ATH\r");
            break;
        default:
            break;
        }
    }

    return NULL;
}

void* sim900_proc_handler(void *arg)
{
    msg_t msg_q[queue_handler_size];
    uint8_t data, state = 0;
    char pattern_vc[] = "+CLIP: \"";
    char pattern_vc_end[] = "NO CARRIER\r\n";
    char pattern_vc_no_ans[] = "NO ANSWER\r\n";
    char pattern_vc_end_bs[] = "BUSY\r\n";
    char pattern_sms[] = "+CMT: \"";
    char pattern_ok[] = "OK\r\n";
    char *ptr_sms_tmp = pattern_sms;
    char *ptr_vc_tmp = pattern_vc;
    char *ptr_vc_bs_tmp = pattern_vc_end_bs;
    char *pn_ptr = phone_number;
    char *ptr_vc_no_tmp = pattern_vc_no_ans;
    char *ptr_ok_tmp = pattern_ok;

    printf("==== sim900_proc thread ====\n");

    state = 2;
    /* Initialize msg queue */
    msg_init_queue(msg_q, queue_handler_size);

    msg_t msg;
    while (1) {
        msg_receive(&msg);
        if (msg.type == UART_ID) {
            data = (uint8_t) msg.content.value;

            switch (state) {
            case 0: //sms
                if (data != (uint8_t) '"') {
                    *pn_ptr++ = data;
                }
                break;
            case 1: //voice call
                if (data != (uint8_t) '"') {
                    *pn_ptr++ = data;
                } else {
                    pn_ptr = phone_number;
                    state = 3;
                    msg.type = VC_ID;
                    msg.content.ptr = phone_number;
                    msg_send(&msg, mbdemo_ns::thread_pid[0], false);
                    ptr_vc_tmp = pattern_vc_end;
                }
                break;
            case 2:
                if (data == (uint8_t) *ptr_sms_tmp) {
                    if (*ptr_sms_tmp == '"') {
                        state = 0;
                        ptr_sms_tmp = pattern_sms;
                        ptr_vc_tmp = pattern_vc;
                    }
                    ptr_sms_tmp++;
                } else {
                    ptr_sms_tmp = pattern_sms;
                }
                if (data == (uint8_t) *ptr_vc_tmp) {
                    if (*ptr_vc_tmp == '"') {
                        state = 1;
                        ptr_sms_tmp = pattern_sms;
                        ptr_vc_tmp = pattern_vc;
                    }
                    ptr_vc_tmp++;
                } else {
                    ptr_vc_tmp = pattern_vc;
                }
                if (data == (uint8_t) *ptr_vc_bs_tmp) {
                    if (*ptr_vc_bs_tmp == '\r') {
                        ptr_sms_tmp = pattern_sms;
                        ptr_vc_tmp = pattern_vc;
                        ptr_vc_bs_tmp = pattern_vc_end_bs;
                        ptr_vc_no_tmp = pattern_vc_no_ans;
                        msg.type = VC_END_BS_ID;
                        msg_send(&msg, mbdemo_ns::thread_pid[0], false);
                    }
                    ptr_vc_bs_tmp++;
                } else {
                    ptr_vc_bs_tmp = pattern_vc_end_bs;
                }
                if (data == (uint8_t) *ptr_vc_no_tmp) {
                    if (*ptr_vc_no_tmp == '\r') {
                        ptr_sms_tmp = pattern_sms;
                        ptr_vc_tmp = pattern_vc;
                        ptr_vc_bs_tmp = pattern_vc_end_bs;
                        ptr_vc_no_tmp = pattern_vc_no_ans;
                        msg.type = VC_END_NO_ID;
                        msg_send(&msg, mbdemo_ns::thread_pid[0], false);
                    }
                    ptr_vc_no_tmp++;
                } else {
                    ptr_vc_no_tmp = pattern_vc_no_ans;
                }
                break;
            case 3:
                if (data == (uint8_t) *ptr_vc_tmp) {
                    if (*ptr_vc_tmp == '\r') {
                        state = 2;
                        ptr_sms_tmp = pattern_sms;
                        ptr_vc_tmp = pattern_vc;
                        msg.type = VC_END_ID;
                        msg_send(&msg, mbdemo_ns::thread_pid[0], false);
                    }
                    ptr_vc_tmp++;
                } else {
                    ptr_vc_tmp = pattern_vc_end;
                }

                if (data == (uint8_t)*ptr_ok_tmp)
                {
                    if (*ptr_ok_tmp == '\n') state = 2;
                    ptr_ok_tmp++;
                }
                else ptr_ok_tmp = pattern_ok;
                break;
            default:
                break;
            }
        }
    }

    return NULL;
}

void usart2_irq()
{
    uint8_t buf = USART_ReceiveData(USART2);

    msg_t msg;
    msg.type = UART_ID;
    msg.content.value = 0;
    msg.content.value = buf;
    msg_send(&msg, mbdemo_ns::thread_pid[2], false);
}

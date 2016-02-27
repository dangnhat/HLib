/*
 * sim900.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: Nguyen Dinh Trung Truc
 */
extern "C" {
#include "vtimer.h"
}
#include "sim900.h"
#include <stdio.h>
#include <string.h>

using namespace gpio_ns;

const gpio_params_t nRst_gpio = { port_A, 0, out_push_pull, speed_50MHz };

const gpio_params_t nPw_gpio = { port_C, 5, out_push_pull, speed_50MHz };

rb_handle_t rxBuffer;

const char ctrz = 0x1A;

void sim900_delay_us(uint32_t us)
{
    vtimer_usleep(us);
}

void sim900_power_init()
{
    gpio nRst_pin, nPw_pin;

    nRst_pin.gpio_init(&nRst_gpio);
    nPw_pin.gpio_init(&nPw_gpio);

    nPw_pin.gpio_set();
    sim900_delay_us(100000);
    nPw_pin.gpio_reset();

    sim900_delay_us(500000);

    nRst_pin.gpio_set();
    sim900_delay_us(1000000);
    nRst_pin.gpio_reset();

    sim900_delay_us(1000000);
}

void sim900_Init()
{
    printf("\nInitializing SIM900 (about 5 seconds) ...\n");

    sim900_power_init();

    /* Turn echo off */
    MB1_USART2.Print("ATE0\r");
    sim900_delay_us(1000000);

    /* SMS in text mode */
    MB1_USART2.Print("AT+CMGF=1\r");
    sim900_delay_us(1000000);

    /* Set how the modem will response when a SMS is received */
    MB1_USART2.Print("AT+CNMI=1,2,0,0,0\r");
    sim900_delay_us(1000000);

    /* Show caller number */
    MB1_USART2.Print("AT+CLIP=1\r");
    sim900_delay_us(1000000);

    /* report feature */
    MB1_USART2.Print("AT+CSMP=17,167,0,240\r");
    sim900_delay_us(1000000);

    printf("DONE !\n");
}

void sim900_makeCall(uint8_t *number)
{
    uint8_t output[20] = "";

    sprintf((char *)output, "ATD%s;\r", (char *)number);

    MB1_USART2.Print(output);
}

void sim900_sendMsg(uint8_t *number, uint8_t *msg)
{
    uint8_t sendNum[50];

    sprintf((char *)sendNum, "AT+CMGS=\"%s\"\r", (char *)number);

    MB1_USART2.Print(sendNum);
    sim900_delay_us(1000000);

    MB1_USART2.Print(msg);
    sim900_delay_us(1000000);

    MB1_USART2.Print(ctrz);

}

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

extern "C" {
#include "vtimer.h"
#include "thread.h"
}
#include "mbdemo.h"

/* configurable variables */
const int16_t stack_size = 1550;
const char thread_name[mbdemo_ns::max_threads][8] = { "SIM900A", "BTN-LCD", };

char stack[mbdemo_ns::max_threads][stack_size];

int main(void)
{
    mbdemo_init();

    /* create threads */
    mbdemo_ns::thread_pid[0] = thread_create(stack[0], stack_size,
    PRIORITY_MAIN - 1, CREATE_STACKTEST, sim900a_handler, NULL, "SIM900A");

    mbdemo_ns::thread_pid[1] = thread_create(stack[1], stack_size,
    PRIORITY_MAIN - 1, CREATE_STACKTEST, btn_lcd_handler, NULL, "BTN-LCD");

    /* Run shell */
    ha_shell_irun(NULL);
}


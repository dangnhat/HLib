/**
 * @file os_dependent_code.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-Jul-2015
 */

extern "C" {
#include "vtimer.h"
#include "thread.h"
#include "posix_io.h"
#include "board_uart0.h"
}

#include "MB1_System.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

namespace testing_ns {
/* Communications global vars */
bool esc_pressed = false;
}

using namespace testing_ns;

/* esc_waiting_thread private data and function */
static kernel_pid_t esc_waiting_thread_pid = 0;
static const uint16_t esc_waiting_thread_stacksize = 1024;
static char sec_waiting_thread_stack[esc_waiting_thread_stacksize];
static void * esc_waiting_thread_func(void *);

static char esc_char_value = 27; /* ESC character */

/*----------------------------------------------------------------------------*/
int16_t esc_waiting_thread_start(void)
{
    /* Create esc_waiting_thread */
    esc_waiting_thread_pid = thread_create(sec_waiting_thread_stack,
        esc_waiting_thread_stacksize, PRIORITY_MAIN - 1,
        CREATE_STACKTEST | CREATE_SLEEPING,
        esc_waiting_thread_func,
        NULL, "esc waiting thread");

    if (esc_waiting_thread_pid < 0) {
        HA_DEBUG("esc_waiting_thread_start: failed to create esc_waiting_thread.\n");
        return -1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
static void * esc_waiting_thread_func(void *)
{
    char c = 0;
    int16_t res = 0;

    while (1) {
        /* open uart0 */
        if (posix_open(uart0_handler_pid, 0) >= 0) {
            HA_DEBUG("esc_waiting_thread_func: open uart0_handler successfully!\n");

            while (1) {
                res = posix_read(uart0_handler_pid, &c, 1);
                if (res > 0 && c == esc_char_value) {
                    HA_DEBUG("esc_waiting_thread_func: esc was pressed.\n");
                    esc_pressed = true;
                    break;
                }
            }

            /* Close uart0_handler and go to sleep */
            HA_DEBUG("esc_waiting_thread_func: close uart0 and go back to sleep.\n");
            if (posix_close(uart0_handler_pid) < 0) {
                HA_DEBUG("esc_waiting_thread_func: error occurred when closing uart0_handler.\n");
            }
            thread_sleep();
        }
        else {
            HA_DEBUG("esc_waiting_thread_func: error occurred when opening uart0_handler!\n");
            thread_sleep();
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/
int16_t start_waiting_esc_character(void)
{
    esc_pressed = false;

    if (posix_close(uart0_handler_pid) < 0) {
        HA_DEBUG("start_waiting_esc_character: error occurred when closing uart0_handler.\n");
        return -1;
    }
    else {
        HA_DEBUG("start_waiting_esc_character: close uart0_handler successfully.\n");
    }

    if (thread_wakeup(esc_waiting_thread_pid) != 1) {
        HA_DEBUG("start_waiting_esc_character: failed to wake up esc_waiting_thread.\n");
        return -1;
    }
    else {
        HA_DEBUG("start_waiting_esc_character: wakeup esc_waiting_thread successfully.\n");
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int16_t stop_waiting_esc_character(void)
{
    esc_pressed = false;

    if (posix_open(uart0_handler_pid, 0) < 0) {
        HA_DEBUG("stop_waiting_esc_character: error occurred when reopening uart0_handler.\n");
        return -1;
    }
    else {
        HA_DEBUG("stop_waiting_esc_character: reopening uart0_handler successfully.\n");
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
void testing_delay_us(uint32_t us)
{
    vtimer_usleep(us);
}

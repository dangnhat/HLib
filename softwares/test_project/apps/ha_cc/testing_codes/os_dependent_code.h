/**
 * @file os_dependent_code.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-Jul-2015
 * @brief This is header holding common functions' definitions for OS-dependent code.
 */

#ifndef OS_DEPENDENT_CODE_H_
#define OS_DEPENDENT_CODE_H_

/* Includes */
#include "stdint.h"
#include "MB1_System.h"

extern "C" {
}

/* Constants and definitions */
namespace testing_ns {

extern bool esc_pressed;

}

/**
 * @brief   Start the esc_waiting_thread.
 *
 * @return      -1 if error.
 */
int16_t esc_waiting_thread_start(void);

/**
 * @brief   Wake up the esc_waiting_thread and start waiting for the ESC character.
 *
 * @return      -1 if error.
 */
int16_t start_waiting_esc_character(void);

/**
 * @brief   Restore the resources used by esc_waiting_thread after esc was pressed.
 *          This function should be called before return in the testing shell command,
 *          where start_waiting_esc_character was called.
 *
 * @return      -1 if error.
 */
int16_t stop_waiting_esc_character(void);

/**
 * @brief   Delay in us.
 *
 * @param[in]   us, us to delay.
 */
void testing_delay_us(uint32_t us);

/**
 * @brief   Wait to receive usart data within a time period.
 *
 * @param[in]   usart_p, pointer to a usart object.
 * @param[in]   timeout, timeount value (in seconds).
 * @param[out]  buffer,  buffer to hold data.
 * @param[in]   len, length of the allocated buffer.
 */
void wait_uart_with_timeout(serial_t *usart_p, uint16_t timeout,
        char *buffer, uint16_t len);

#endif /* OS_DEPENDENT_CODE_H_ */

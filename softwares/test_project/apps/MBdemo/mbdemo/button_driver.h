/**
 * @file button_switch_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for button & switch device instance in HA system.
 */
#ifndef __HA_BUTTON_SWITCH_DRIVER_H_
#define __HA_BUTTON_SWITCH_DRIVER_H_
#include <stdlib.h>
/* RIOT's include */
extern "C" {
#include "thread.h"
#include "msg.h"
}

#include "MB1_GPIO.h"

namespace btn_ns {
typedef enum
    : uint8_t {
        /* button status */
        btn_no_pressed = 0x00,
    btn_pressed = 0x01,
    btn_on_hold = 0x02
} btn_status_t;
}

class button {
public:
    button(void);
    ~button(void);

    void set_btn_id(uint8_t id);
    uint8_t get_btn_id(void);

    void button_configure(gpio_ns::gpio_params_t *gpio_config_params);

    /**
     *
     */
    btn_ns::btn_status_t get_status(void);

    /**
     *
     */
    bool is_changed_status(void);

    /**
     *
     */
    void btn_processing(void);

    kernel_pid_t get_pid(void);
private:
    btn_ns::btn_status_t current_status, old_status;

    uint8_t new_state_reg_1;
    uint8_t new_state_reg_2;
    uint8_t new_state_reg_3;
    uint8_t old_state_reg;

    uint8_t btn_id;

    uint16_t hold_time_count;

    gpio gpio_dev;

    kernel_pid_t thread_pid;

    void assign_btn(void);
};

void btn_callback_timer_isr(void);

#endif //__HA_BUTTON_SWITCH_DRIVER_H_

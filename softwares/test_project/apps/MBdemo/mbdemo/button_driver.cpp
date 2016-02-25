/**
 * @file button_switch_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for button device instance in HA system.
 */
#include "button_driver.h"
#include <stdio.h>
using namespace btn_ns;
using namespace gpio_ns;

/* configurable variables */
const uint8_t btn_active_state = 0; //active low-level
const static uint8_t timer_period = 1; //ms
const uint8_t btn_sampling_time_cycle = 10 / timer_period; //sampling every 10ms (tim6_period = 1ms)
const uint16_t btn_hold_time = 1 * 1000 / btn_sampling_time_cycle; //btn is on hold after holding 1s.

static uint8_t time_cycle_count = 0;

#define MAX_BTNS 2
static button* btn_list[MAX_BTNS];
static bool btnListInited = false;

static void init_btn_list(void);

static void init_btn_list(void)
{
    for (uint8_t i = 0; i < MAX_BTNS; i++) {
        btn_list[i] = NULL;
    }
}

void button::assign_btn(void)
{
    for (uint8_t i = 0; i < MAX_BTNS; i++) {
        if (btn_list[i] == NULL) {
            btn_list[i] = this;
            break;
        }
    }
}

button::button(void)
{
    this->new_state_reg_1 = !btn_active_state;
    this->new_state_reg_2 = !btn_active_state;
    this->new_state_reg_3 = !btn_active_state;
    this->old_state_reg = !btn_active_state;

    this->btn_id = 0;

    this->hold_time_count = 0;

    this->current_status = btn_no_pressed;
    this->old_status = btn_no_pressed;

    if (!btnListInited) {
        init_btn_list();
    }

    this->thread_pid = thread_getpid();
}

void button::set_btn_id(uint8_t id) {
    this->btn_id = id;
}

uint8_t button::get_btn_id(void) {
    return this->btn_id;
}

void button::button_configure(gpio_params_t *gpio_config_params)
{
    gpio_dev.gpio_init(gpio_config_params);
    assign_btn();
}

btn_status_t button::get_status(void)
{
    btn_status_t status = current_status;

    if (current_status == btn_pressed) {
        current_status = btn_no_pressed;
    }

    old_status = current_status;

    return status;
}

bool button::is_changed_status(void)
{
    if (current_status != old_status) {
        return true;
    }
    return false;
}

void button::btn_processing(void)
{
    /* sampling */
    new_state_reg_3 = new_state_reg_2;
    new_state_reg_2 = new_state_reg_1;
    new_state_reg_1 = gpio_dev.gpio_read();

    /* update hold_time_count value */
    if (hold_time_count != 0) {
        hold_time_count--;
        if (hold_time_count == 0) { //time out, button is hold.
            current_status = btn_on_hold;
        }
    }

    /* processing */
    if ((new_state_reg_1 == new_state_reg_2)
            && (new_state_reg_2 == new_state_reg_3)) { //new stable state

        if (new_state_reg_1 != old_state_reg) { //change state

            old_state_reg = new_state_reg_1;

            if (new_state_reg_1 == btn_active_state) { //change from inactive->active
                hold_time_count = btn_hold_time; //set time out value
            } else { //change from active->inactive
                if (hold_time_count > 0) {  //button is pressed
                    current_status = btn_pressed;
                } else {    //button is un-hold
                    current_status = btn_no_pressed;
                }
                hold_time_count = 0;
            }
        } //end if()
        else { //not change state
            if (new_state_reg_1 == !btn_active_state) { //not activate -> btn isn't pressed
                current_status = btn_no_pressed;
            }
        }
    } // end if()
}

void btn_callback_timer_isr(void)
{
    time_cycle_count = time_cycle_count + 1;

    if (time_cycle_count == btn_sampling_time_cycle) {
        time_cycle_count = 0;
        for (uint8_t i = 0; i < MAX_BTNS; i++) {
            if (btn_list[i] != NULL) {
                btn_list[i]->btn_processing();
                if (btn_list[i]->is_changed_status()) {
                    printf("ok\n");
                    msg_t msg;
                    msg.type = btn_list[i]->get_btn_id();
                    msg.content.value = (uint32_t) btn_list[i]->get_status();
                    kernel_pid_t pid = btn_list[i]->get_pid();
                    if (pid == KERNEL_PID_UNDEF) {
                        return;
                    }
                    msg_send(&msg, pid, false);
                }
            }
        } //end for()
    } //end if()
}

kernel_pid_t button::get_pid(void)
{
    return this->thread_pid;
}

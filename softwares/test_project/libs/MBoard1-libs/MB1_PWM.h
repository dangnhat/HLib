/**
 * @file MB1_PWM.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 16-10-2014
 * @brief This is header file for PWM hardware for MBoard-1.
 * @How to use:
 *  -Declare an pwm-class instance.
 *  -Initialize GPIO as alternative function output for TIM channel.
 *  -Enter value to an "pwm_params_t"
 *  -Pass above struct into pwm_init() method to initialize PWM.
 *  Note: TIM6&7 have no PWM hardware module.
 *        The number of PWM channels depends on used TIM.
 */
#ifndef __MB1_PWM_H_
#define __MB1_PWM_H_

#include "MB1_Glb.h"

namespace pwm_ns {
typedef enum {
    adv_timer1 = 0, //advanced-control timer
    adv_timer8 = 1, //advanced-control timer
    gp_timer2 = 2,  //general-purpose timer
    gp_timer3 = 3,  //general-purpose timer
    gp_timer4 = 4,  //general-purpose timer
    gp_timer5 = 5   //general-purpose timer
} pwm_timer_t;

typedef struct {
    pwm_timer_t timer;
    uint16_t pwm_mode;
    uint8_t pwm_channel;
    uint16_t period;
    uint16_t prescaler;
    uint16_t pulse_width;
} pwm_params_t;
}

class pwm_hw {
public:
    pwm_hw(void);

    /**
     * @brief Initialize PWM.
     *
     * @param[in] pwm_params Contains user parameter to initialize PWM.
     *
     * @return true if success, false otherwise.
     */
    bool pwm_init(pwm_ns::pwm_params_t *pwm_params);

    /**
     * @brief Set up period of PWM.
     *
     * @param[in] period
     */
    void pwm_period_setup(uint16_t period);

    /**
     * @brief Set up prescaler of PWM.
     *
     * @param[in] prescaler 0->65535.
     */
    void pwm_prescaler_setup(uint16_t prescaler);

    /**
     * @brief Set up pulse width.
     *
     * @params[in] pulse_width
     */
    void pwm_pulse_width_setup(uint16_t pulse_width);

    /**
     * @brief enable output channel
     */
    void pwm_oc_enable(void);

    /**
     * @brief disable output channel
     */
    void pwm_oc_disable(void);
protected:
    uint16_t period;
    uint16_t prescaler;
private:
    uint8_t timer;
    uint8_t channel;
    uint16_t mode;

    /**
     * @brief Initialize pwm base on timer.
     */
    void timer_base_init(void);

    /**
     * @brief Initialize output channel.
     *
     * @param[in] timer_pulse Pulse width
     */
    void output_channel_init(uint16_t timer_pulse);
};

#endif //__MB1_PWM_H_

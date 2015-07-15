/**
 * @file MB1_PWM.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 16-10-2014
 * @brief This is source file for PWM hardware for MBoard-1.
 */
#include "MB1_PWM.h"

using namespace pwm_ns;

TIM_TypeDef *timer_x[] = { TIM1, TIM8, TIM2, TIM3, TIM4, TIM5 };

pwm_hw::pwm_hw(void)
{
    this->timer = 0; //TIM1 as default.
    this->channel = 1;
    this->mode = TIM_OCMode_PWM1;
    this->period = 665;
    this->prescaler = 1;
}

bool pwm_hw::pwm_init(pwm_params_t *pwm_params)
{
    uint32_t timer_rcc;
    bool advanced_timer = false;

    /* save internal value */
    this->channel = pwm_params->pwm_channel;
    this->period = pwm_params->period;
    this->prescaler = pwm_params->prescaler;
    this->mode = pwm_params->pwm_mode;
    this->timer = (uint8_t) pwm_params->timer;

    switch (pwm_params->timer) {
    case adv_timer1:
        timer_rcc = RCC_APB2Periph_TIM1;
        advanced_timer = true;
        break;
    case adv_timer8:
        timer_rcc = RCC_APB2Periph_TIM8;
        advanced_timer = true;
        break;
    case gp_timer2:
        timer_rcc = RCC_APB1Periph_TIM2;
        break;
    case gp_timer3:
        timer_rcc = RCC_APB1Periph_TIM3;
        break;
    case gp_timer4:
        timer_rcc = RCC_APB1Periph_TIM4;
        break;
    case gp_timer5:
        timer_rcc = RCC_APB1Periph_TIM5;
        break;
    default:
        return false;
    }

    /* Timer RCC configuration */
    if (advanced_timer) {
        RCC_APB2PeriphClockCmd(timer_rcc, ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(timer_rcc, ENABLE);
    }

    /* Timer base configuration */
    timer_base_init();

    /* PWMx mode configuration: Channel y */
    output_channel_init(pwm_params->pulse_width);

    TIM_ARRPreloadConfig(timer_x[timer], ENABLE);

    /* enable timer counter */
    TIM_Cmd(timer_x[timer], ENABLE);

    return true;
}

void pwm_hw::pwm_period_setup(uint16_t period)
{
    this->period = period; //update period.
    timer_base_init();
}

void pwm_hw::pwm_prescaler_setup(uint16_t prescaler)
{
    this->prescaler = prescaler; //update prescaler
    timer_base_init();
}

void pwm_hw::pwm_pulse_width_setup(uint16_t pulse_width)
{
    output_channel_init(pulse_width);
}

void pwm_hw::timer_base_init(void)
{
    TIM_TimeBaseInitTypeDef time_base_init_struct;

    time_base_init_struct.TIM_Period = this->period;
    time_base_init_struct.TIM_Prescaler = this->prescaler;
    time_base_init_struct.TIM_ClockDivision = 0;
    time_base_init_struct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(timer_x[timer], &time_base_init_struct);
}

void pwm_hw::output_channel_init(uint16_t timer_pulse)
{
    TIM_OCInitTypeDef oc_init_struct;

    oc_init_struct.TIM_OCMode = this->mode;
    oc_init_struct.TIM_Pulse = timer_pulse;
    oc_init_struct.TIM_OutputState = TIM_OutputState_Enable;
    oc_init_struct.TIM_OCPolarity = TIM_OCPolarity_High;
    switch (this->channel) {
    case 1:
        TIM_OC1Init(timer_x[timer], &oc_init_struct);
        break;
    case 2:
        TIM_OC2Init(timer_x[timer], &oc_init_struct);
        break;
    case 3:
        TIM_OC3Init(timer_x[timer], &oc_init_struct);
        break;
    case 4:
        TIM_OC4Init(timer_x[timer], &oc_init_struct);
        break;
    default:
        break;
    }

    pwm_oc_enable();
}

void pwm_hw::pwm_oc_enable(void)
{
    switch (this->channel) {
    case 1:
        TIM_OC1PreloadConfig(timer_x[timer], TIM_OCPreload_Enable);
        break;
    case 2:
        TIM_OC2PreloadConfig(timer_x[timer], TIM_OCPreload_Enable);
        break;
    case 3:
        TIM_OC3PreloadConfig(timer_x[timer], TIM_OCPreload_Enable);
        break;
    case 4:
        TIM_OC4PreloadConfig(timer_x[timer], TIM_OCPreload_Enable);
        break;
    default:
        break;
    }
}

void pwm_hw::pwm_oc_disable(void)
{
    switch (this->channel) {
    case 1:
        TIM_OC1PreloadConfig(timer_x[timer], TIM_OCPreload_Disable);
        break;
    case 2:
        TIM_OC2PreloadConfig(timer_x[timer], TIM_OCPreload_Disable);
        break;
    case 3:
        TIM_OC3PreloadConfig(timer_x[timer], TIM_OCPreload_Disable);
        break;
    case 4:
        TIM_OC4PreloadConfig(timer_x[timer], TIM_OCPreload_Disable);
        break;
    default:
        break;
    }
}

/**
 * @file MB1_GPIO.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 08-10-2014
 * @brief This is source file for manage GPIO and its EXTI for MBoard-1.
 *
 */
#include "MB1_GPIO.h"

using namespace gpio_ns;

GPIO_TypeDef *gpio_x[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG };

const uint32_t periph_rcc[] = { RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB,
RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD, RCC_APB2Periph_GPIOE,
RCC_APB2Periph_GPIOF, RCC_APB2Periph_GPIOG };

gpio::gpio(void)
{
    this->in_mode = true;
}

bool gpio::gpio_init(const gpio_params_t *gpio_params)
{
    GPIO_InitTypeDef gpio_init_struct;

    this->port_source = (uint8_t) gpio_params->port;
    this->pin_source = gpio_params->pin;

    /* Check params */
    if (this->port_source > 6 || this->pin_source > 15) {
        return false;
    }

    /* Enable peripheral clock */
    RCC_APB2PeriphClockCmd(periph_rcc[port_source], ENABLE);
    if (gpio_params->mode == af_open_drain
            || gpio_params->mode == af_push_pull) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    }

    if (gpio_params->mode == out_open_drain
            || gpio_params->mode == out_push_pull) {
        in_mode = false;
    }

    /* Initialize GPIO */
    gpio_init_struct.GPIO_Pin = (uint16_t) 1 << pin_source;
    gpio_init_struct.GPIO_Mode = (GPIOMode_TypeDef) gpio_params->mode;
    switch(gpio_params->gpio_speed) {
    case speed_2MHz:
        gpio_init_struct.GPIO_Speed = GPIO_Speed_2MHz;
        break;
    case speed_10MHz:
        gpio_init_struct.GPIO_Speed = GPIO_Speed_10MHz;
        break;
    case speed_50MHz:
        gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
        break;
    default:
        return false;
    }

    GPIO_Init(gpio_x[port_source], &gpio_init_struct);

    return true;
}

uint8_t gpio::gpio_read(void)
{
    if (in_mode) {
        return GPIO_ReadInputDataBit(gpio_x[port_source],
                ((uint16_t) 1 << pin_source));
    } else {
        return GPIO_ReadOutputDataBit(gpio_x[port_source],
                ((uint16_t) 1 << pin_source));
    }
}

void gpio::gpio_set(void)
{
    GPIO_SetBits(gpio_x[port_source], ((uint16_t) 1 << pin_source));
}

void gpio::gpio_reset(void)
{
    GPIO_ResetBits(gpio_x[port_source], ((uint16_t) 1 << pin_source));
}

void gpio::gpio_assign_value(uint8_t value)
{
    if (value == 1) {
        gpio_set();
    }
    else {
        gpio_reset();
    }
}

bool gpio::exti_init(exti_trigger_t trigger)
{
    /* Save trigger state */
    this->trigger = trigger;

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /* Connect EXTI Line to its pin */
    GPIO_EXTILineConfig(port_source, pin_source);

    /* Initialize EXTI */
    exti_line_init(this->trigger, ENABLE);

    nvic_init(0x0F, 0x0F);

    return true;
}

void gpio::exti_line_enable(void)
{
    exti_line_init(this->trigger, ENABLE);
}

void gpio::exti_line_disable(void)
{
    exti_line_init(this->trigger, DISABLE);
}

void gpio::exti_trigger_setup(exti_trigger_t trigger)
{
    this->trigger = trigger;
    exti_line_init(this->trigger, ENABLE);
}

void gpio::exti_priority_setup(uint8_t preemption, uint8_t sub)
{
    nvic_init(preemption, sub);
}

void gpio::exti_line_init(exti_trigger_t trigger, FunctionalState new_state) {
    EXTI_InitTypeDef exti_init_struct;

    exti_init_struct.EXTI_Line = (uint16_t) 1 << pin_source;
    exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init_struct.EXTI_Trigger = (EXTITrigger_TypeDef) this->trigger;
    exti_init_struct.EXTI_LineCmd = new_state;
    EXTI_Init(&exti_init_struct);
}

void gpio::nvic_init(uint8_t preemption, uint8_t sub)
{
    NVIC_InitTypeDef nvic_init_struct;

    switch (pin_source) {
    case 0:
        nvic_init_struct.NVIC_IRQChannel = EXTI0_IRQn;
        break;
    case 1:
        nvic_init_struct.NVIC_IRQChannel = EXTI1_IRQn;
        break;
    case 2:
        nvic_init_struct.NVIC_IRQChannel = EXTI2_IRQn;
        break;
    case 3:
        nvic_init_struct.NVIC_IRQChannel = EXTI3_IRQn;
        break;
    case 4:
        nvic_init_struct.NVIC_IRQChannel = EXTI4_IRQn;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        nvic_init_struct.NVIC_IRQChannel = EXTI9_5_IRQn;
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
        break;
    default:
        break;
    }
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = preemption;
    nvic_init_struct.NVIC_IRQChannelSubPriority = sub;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
}

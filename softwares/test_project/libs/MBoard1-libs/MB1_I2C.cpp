/*
 * MB1_I2C.cpp
 *
 *  Created on: Aug 25, 2015
 *      Author: nvhien1992
 */
#include <stdio.h>
#include "MB1_I2C.h"

#define COUNT_TIME_OUT (1000)

using namespace I2C_ns;

I2C_TypeDef* I2Cs[num_of_I2Cs] = { I2C1, I2C2 };

uint32_t RCC_I2Cs[num_of_I2Cs] = { RCC_APB1Periph_I2C1, RCC_APB1Periph_I2C2 };

//GPIO_TypeDef* SDA_ports[num_of_I2Cs] = { GPIOB, GPIOB};
const uint16_t SDA_pins[num_of_I2Cs] = { GPIO_Pin_7, GPIO_Pin_11 };

//GPIO_TypeDef* SCL_ports[num_of_I2Cs] = { GPIOB, GPIOB };
const uint16_t SCL_pins[num_of_I2Cs] = { GPIO_Pin_6, GPIO_Pin_10 };

I2C::I2C(uint8_t usued_i2c)
{
    this->used_i2c = usued_i2c;
}

void I2C::reinit(I2C_InitTypeDef *init_structure)
{
    /* I2C clock enable */
    RCC_APB1PeriphClockCmd(RCC_I2Cs[this->used_i2c - 1], ENABLE);

    /* GPIOB clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* I2C SDA and SCL configuration */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_pins[this->used_i2c - 1]
            | SCL_pins[this->used_i2c - 1];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_ClockSpeed = init_structure->I2C_ClockSpeed;
    I2C_InitStructure.I2C_Mode = init_structure->I2C_Mode;
    I2C_InitStructure.I2C_DutyCycle = init_structure->I2C_DutyCycle;
    I2C_InitStructure.I2C_Ack = init_structure->I2C_Ack;
    I2C_InitStructure.I2C_AcknowledgedAddress =
            init_structure->I2C_AcknowledgedAddress;
    I2C_InitStructure.I2C_OwnAddress1 = init_structure->I2C_OwnAddress1;
    I2C_Init(I2Cs[this->used_i2c - 1], &I2C_InitStructure);

    /* enable I2C */
    I2C_Cmd(I2Cs[this->used_i2c - 1], ENABLE);
}

uint8_t I2C::get_used_i2c(void)
{
    return this->used_i2c;
}

void I2C::deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* disable I2C */
    I2C_Cmd(I2Cs[this->used_i2c], DISABLE);

    /* disable clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_I2Cs[this->used_i2c], DISABLE);

    /* config IO pins into IN_FLOATING mode */
    GPIO_InitStruct.GPIO_Pin = SCL_pins[this->used_i2c]
            | SDA_pins[this->used_i2c];
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

bool I2C::master_send_to(uint16_t slave_7b_addr, uint8_t *send_buff,
        uint16_t size, bool stop_signal)
{
    uint16_t waiting_time_count = COUNT_TIME_OUT;

    /* generate start signal */
    I2C_GenerateSTART(I2Cs[this->used_i2c - 1], ENABLE);
    /* check start bit flag (EV5) */
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* send slave address with write command */
    I2C_Send7bitAddress(I2Cs[this->used_i2c - 1],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Transmitter);

    /* check master is now in Tx mode (EV6) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    for (uint16_t i = 0; i < size; i++) {
        /* send data */
        I2C_SendData(I2Cs[this->used_i2c - 1], send_buff[i]);
        /* wait for byte shifted completely (EV8) */
        waiting_time_count = COUNT_TIME_OUT;
        while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
        I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
            if ((waiting_time_count--) == 0) {
                return false;
            }
        }
    }

    /* check sending is complete (EV8_2) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    if (stop_signal) {
        /* generate stop signal */
        I2C_GenerateSTOP(I2Cs[this->used_i2c - 1], ENABLE);
    }

    return true;
}

bool I2C::master_receive_from(uint16_t slave_7b_addr, uint8_t *recv_buff,
        uint16_t size)
{
    uint16_t waiting_time_count = COUNT_TIME_OUT;

    /* re-enable ACK bit disabled in last call */
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c - 1], ENABLE);

    /* generate START signal a second time (Re-Start) */
    I2C_GenerateSTART(I2Cs[this->used_i2c - 1], ENABLE);
    /* check start bit flag (EV5) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* send address for read */
    I2C_Send7bitAddress(I2Cs[this->used_i2c - 1],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Receiver);
    /* check master is now in Rx mode (EV6) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    for (uint16_t i = 0; i < size; i++) {
        /* receive all registers */
        waiting_time_count = COUNT_TIME_OUT;
        while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
        I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            if ((waiting_time_count--) == 0) {
                return false;
            }
        }
        recv_buff[i] = I2C_ReceiveData(I2Cs[this->used_i2c - 1]);
    }

    /* enable NACK bit */
    I2C_NACKPositionConfig(I2Cs[this->used_i2c - 1], I2C_NACKPosition_Current);
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c - 1], DISABLE);

    /* generate STOP signal */
    I2C_GenerateSTOP(I2Cs[this->used_i2c - 1], ENABLE);

    return true;
}

bool I2C::master_receive_from(uint16_t slave_7b_addr, uint8_t slave_register,
        uint8_t *recv_buff, uint16_t size)
{
    uint16_t waiting_time_count = COUNT_TIME_OUT;

    /* generate start signal */
    I2C_GenerateSTART(I2Cs[this->used_i2c - 1], ENABLE);
    /* check start bit flag (EV5) */
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* send slave address with write command */
    I2C_Send7bitAddress(I2Cs[this->used_i2c - 1],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Transmitter);
    /* check master is now in Tx mode (EV6) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* set slave internal register */
    I2C_SendData(I2Cs[this->used_i2c - 1], slave_register);
    /* wait for byte shifted completely (EV8) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* re-enable ACK bit disabled in last call */
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c - 1], ENABLE);

    /* generate START signal a second time (Re-Start) */
    I2C_GenerateSTART(I2Cs[this->used_i2c - 1], ENABLE);
    /* check start bit flag (EV5) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    /* send address for read */
    I2C_Send7bitAddress(I2Cs[this->used_i2c - 1],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Receiver);
    /* check master is now in Rx mode (EV6) */
    waiting_time_count = COUNT_TIME_OUT;
    while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
    I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        if ((waiting_time_count--) == 0) {
            return false;
        }
    }

    for (uint16_t i = 0; i < size; i++) {
        /* receive all registers */
        waiting_time_count = COUNT_TIME_OUT;
        while (!I2C_CheckEvent(I2Cs[this->used_i2c - 1],
        I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            if ((waiting_time_count--) == 0) {
                return false;
            }
        }
        recv_buff[i] = I2C_ReceiveData(I2Cs[this->used_i2c - 1]);
    }

    /* enable NACK bit */
    I2C_NACKPositionConfig(I2Cs[this->used_i2c - 1], I2C_NACKPosition_Current);
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c - 1], DISABLE);

    /* generate STOP signal */
    I2C_GenerateSTOP(I2Cs[this->used_i2c - 1], ENABLE);

    return true;
}

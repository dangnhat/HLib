/*
 * MB1_I2C.cpp
 *
 *  Created on: Aug 25, 2015
 *      Author: nvhien1992
 */
#include "MB1_I2C.h"

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
    /* enable I2C */
    I2C_Cmd(I2Cs[this->used_i2c], ENABLE);

    /* I2C clock enable */
    RCC_APB1PeriphClockCmd(RCC_I2Cs[this->used_i2c], ENABLE);

    /* I2C1 SDA and SCL configuration */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_pins[this->used_i2c]
            | SCL_pins[this->used_i2c];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_Init(I2Cs[this->used_i2c], init_structure);
}

void I2C::deinit(void)
{

}

void I2C::master_send_to(uint16_t slave_addr, uint8_t *send_buff, uint16_t size,
        bool stop_signal)
{
    /* generate start signal */
    I2C_GenerateSTART(I2Cs[this->used_i2c], ENABLE);
    /* check start bit flag */
    while (I2C_GetFlagStatus(I2Cs[this->used_i2c], I2C_FLAG_SB) == RESET);

    /* send slave address with write command */
    I2C_Send7bitAddress(I2Cs[this->used_i2c], slave_addr << 1,
    I2C_Direction_Transmitter);
    /* check master is now in Tx mode */
    while (I2C_CheckEvent(I2Cs[this->used_i2c],
    I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);

    for (uint16_t i = 0; i < size; i++) {
        /* send data */
        I2C_SendData(I2Cs[this->used_i2c], send_buff[i]);
        /* wait for byte send to complete */
        while (I2C_CheckEvent(I2Cs[this->used_i2c],
        I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
    }

    if (stop_signal) {
        /* generate stop signal */
        I2C_GenerateSTOP(I2Cs[this->used_i2c], ENABLE);
        /* check stop bit flag */
        while (I2C_GetFlagStatus(I2Cs[this->used_i2c], I2C_FLAG_STOPF) == SET);
    }
}

void I2C::master_receive_from(uint16_t slave_addr, uint8_t *recv_buff,
        uint16_t size)
{
    /* re-enable ACK bit disabled in last call */
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c], ENABLE);
    /* check BUSY Flag */
    while (I2C_GetFlagStatus(I2Cs[this->used_i2c], I2C_FLAG_BUSY) == SET);

    /* generate START signal a second time (Re-Start) */
    I2C_GenerateSTART(I2Cs[this->used_i2c], ENABLE);
    /* check start bit flag */
    while (I2C_GetFlagStatus(I2Cs[this->used_i2c], I2C_FLAG_SB) == RESET);

    /* send address for read */
    I2C_Send7bitAddress(I2Cs[this->used_i2c], slave_addr << 1,
            I2C_Direction_Receiver);
    /* check receive mode Flag */
    while (I2C_CheckEvent(I2Cs[this->used_i2c],
            I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);

    for(uint16_t i = 0; i < size; i++) {
        /* receive all registers */
        while (I2C_CheckEvent(I2Cs[this->used_i2c], I2C_EVENT_MASTER_BYTE_RECEIVED)
                == ERROR);
        recv_buff[i] = I2C_ReceiveData(I2Cs[this->used_i2c]);
    }

    /* enable NACK bit */
    I2C_NACKPositionConfig(I2Cs[this->used_i2c], I2C_NACKPosition_Current);
    I2C_AcknowledgeConfig(I2Cs[this->used_i2c], DISABLE);

    /* generate STOP signal */
    I2C_GenerateSTOP(I2Cs[this->used_i2c], ENABLE);
    /* check stop bit flag */
    while (I2C_GetFlagStatus(I2Cs[this->used_i2c], I2C_FLAG_STOPF) == SET);
}

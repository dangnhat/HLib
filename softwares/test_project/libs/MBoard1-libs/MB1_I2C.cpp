/**
 * @file MB1_I2C.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 29-Aug-2015
 * @brief Source files for internal I2C module of MBoard-1.
 */

#include <stdio.h>
#include "MB1_I2C.h"

using namespace i2c_ns;

/* Compile-time configurations */
/* I2C */
static I2C_TypeDef* i2cs[num_of_i2cs] = {I2C1, I2C2};
static const uint32_t rcc_i2cs[num_of_i2cs] = {RCC_APB1Periph_I2C1, RCC_APB1Periph_I2C2};
static void (* rcc_i2c_fp[num_of_i2cs]) (uint32_t, FunctionalState)
        = {RCC_APB1PeriphClockCmd, RCC_APB1PeriphClockCmd};

/* I2C gpios */
static GPIO_TypeDef* SDA_ports[num_of_i2cs] = {GPIOB, GPIOB};
static const uint16_t SDA_pins[num_of_i2cs] = {GPIO_Pin_7, GPIO_Pin_11};
static const uint32_t SDA_RCCs [num_of_i2cs] ={RCC_APB2Periph_GPIOB, RCC_APB2Periph_GPIOB};

static GPIO_TypeDef* SCL_ports[num_of_i2cs] = {GPIOB, GPIOB };
static const uint16_t SCL_pins[num_of_i2cs] = {GPIO_Pin_6, GPIO_Pin_10};
static const uint32_t SCL_RCCs [num_of_i2cs] ={RCC_APB2Periph_GPIOB, RCC_APB2Periph_GPIOB};

/*----------------------------------------------------------------------------*/
i2c::i2c(uint8_t usued_i2c)
{
    this->used_i2c = usued_i2c - 1;
}

/*----------------------------------------------------------------------------*/
void i2c::init(const i2c_ns::i2c_params_t *init_structure)
{
    /* I2C clock enable */
    rcc_i2c_fp[used_i2c](rcc_i2cs[used_i2c], ENABLE);

    /* GPIOB clock enable */
    RCC_APB2PeriphClockCmd(SDA_RCCs[used_i2c] | SCL_RCCs[used_i2c], ENABLE);

    /* I2C SDA and SCL configuration */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_pins[used_i2c];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(SDA_ports[used_i2c], &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SCL_pins[used_i2c];
    GPIO_Init(SCL_ports[used_i2c], &GPIO_InitStructure);

    /* I2C configuration */
    I2C_InitTypeDef  I2C_InitStructure;

    I2C_InitStructure.I2C_Mode = init_structure->mode;
    I2C_InitStructure.I2C_DutyCycle = init_structure->duty_cycle;
    I2C_InitStructure.I2C_OwnAddress1 = init_structure->own_address;
    I2C_InitStructure.I2C_Ack = init_structure->ack;
    I2C_InitStructure.I2C_AcknowledgedAddress = init_structure->own_address;
    I2C_InitStructure.I2C_ClockSpeed = init_structure->clock_speed;

    /* enable I2C */
    I2C_Cmd(i2cs[used_i2c], ENABLE);

    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(i2cs[used_i2c], &I2C_InitStructure);
}

/*----------------------------------------------------------------------------*/
uint8_t i2c::get_used_i2c(void)
{
    return this->used_i2c + 1;
}

/*----------------------------------------------------------------------------*/
void i2c::deinit(void)
{
    /* config IO pins into IN_FLOATING mode */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_pins[used_i2c];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SDA_ports[used_i2c], &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SCL_pins[used_i2c];
    GPIO_Init(SCL_ports[used_i2c], &GPIO_InitStructure);

    /* disable I2C */
    I2C_Cmd(i2cs[used_i2c], DISABLE);

    /* disable clock */
    RCC_APB2PeriphClockCmd(SDA_RCCs[used_i2c] | SCL_RCCs[used_i2c], DISABLE);
    rcc_i2c_fp[used_i2c](rcc_i2cs[used_i2c], DISABLE);
}

/*----------------------------------------------------------------------------*/
void i2c::master_send(uint16_t slave_7b_addr, const uint8_t *send_buff,
        uint16_t size, bool stop_signal)
{
    /* Wait while the bus is busy */
    while(I2C_GetFlagStatus(i2cs[used_i2c], I2C_FLAG_BUSY));

    /* generate start signal */
    I2C_GenerateSTART(i2cs[used_i2c], ENABLE);
    /* check start bit flag (EV5) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_MODE_SELECT));

    /* send slave address with write command */
    I2C_Send7bitAddress(i2cs[used_i2c],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Transmitter);

    /* check master is now in Tx mode (EV6) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    for (uint16_t i = 0; i < size; i++) {
        /* send data */
        I2C_SendData(i2cs[used_i2c], send_buff[i]);
        /* wait for byte shifted completely (EV8) */
        while (!I2C_CheckEvent(i2cs[used_i2c],
                I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    }

    /* check sending is complete (EV8_2) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    if (stop_signal) {
        /* generate stop signal */
        I2C_GenerateSTOP(i2cs[used_i2c], ENABLE);
    }
}

/*----------------------------------------------------------------------------*/
void i2c::master_receive(uint16_t slave_7b_addr, uint8_t slave_register,
        uint8_t *recv_buff, uint16_t size)
{
    /* Wait while the bus is busy */
    while(I2C_GetFlagStatus(i2cs[used_i2c], I2C_FLAG_BUSY));

    /* generate start signal */
    I2C_GenerateSTART(i2cs[used_i2c], ENABLE);
    /* check start bit flag (EV5) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_MODE_SELECT));

    /* send slave address with write command */
    I2C_Send7bitAddress(i2cs[used_i2c],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Transmitter);
    /* check master is now in Tx mode (EV6) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* set slave internal register */
    I2C_SendData(i2cs[used_i2c], slave_register);
    /* wait for byte shifted completely (EV8) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* re-enable ACK bit disabled in last call */
    I2C_AcknowledgeConfig(i2cs[used_i2c], ENABLE);

    /* generate START signal a second time (Re-Start) */
    I2C_GenerateSTART(i2cs[used_i2c], ENABLE);
    /* check start bit flag (EV5) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_MODE_SELECT));

    /* send address for read */
    I2C_Send7bitAddress(i2cs[used_i2c],
            ((uint8_t) slave_7b_addr) << 1,
            I2C_Direction_Receiver);
    /* check master is now in Rx mode (EV6) */
    while (!I2C_CheckEvent(i2cs[used_i2c],
            I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    for (uint16_t i = 0; i < size; i++) {
        /* receive all registers */
        while (!I2C_CheckEvent(i2cs[used_i2c],
                I2C_EVENT_MASTER_BYTE_RECEIVED));

        recv_buff[i] = I2C_ReceiveData(i2cs[used_i2c]);
    }

    /* enable NACK bit */
    I2C_NACKPositionConfig(i2cs[used_i2c], I2C_NACKPosition_Current);
    I2C_AcknowledgeConfig(i2cs[used_i2c], DISABLE);

    /* generate STOP signal */
    I2C_GenerateSTOP(i2cs[used_i2c], ENABLE);
}

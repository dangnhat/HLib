/**
 * @file MB1_I2C.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>
 * @version 1.0
 * @date 29-Aug-2015
 * @brief Header files for internal I2C module of MBoard-1.
 */

#ifndef MB1_I2C_H_
#define MB1_I2C_H_

/* Includes */
#include "MB1_Glb.h"

namespace i2c_ns {
const uint8_t num_of_i2cs = 2;

enum i2c_mode_e: uint16_t {
    i2c = I2C_Mode_I2C,
    smbus_device = I2C_Mode_SMBusDevice,
    smbus_host = I2C_Mode_SMBusHost,
};

enum i2c_fast_mode_duty_cycle_e: uint16_t {
    dc_16_9 = I2C_DutyCycle_16_9,
    dc_2 = I2C_DutyCycle_2,
};

enum i2c_ack_e: uint16_t {
    ack_enable = I2C_Ack_Enable,
    ack_disable = I2C_Ack_Disable,
};

enum i2c_acked_address: uint16_t {
    acked_address_7bit = I2C_AcknowledgedAddress_7bit,
    acked_address_10bit = I2C_AcknowledgedAddress_10bit,
};

typedef struct i2c_params_s {
    uint32_t clock_speed;       /* in Hz, should be <= 400000 */
    uint16_t mode;              /* i2c, smbus_device or smbus_host */
    uint16_t duty_cycle;        /* Only in I2C fast mode, dc_16_9 or dc_2 */
    uint16_t ack;               /* Either ack_enable or ack_disable */
    uint16_t acked_address;     /* Either acked_address_7bit or acked_address_10bit */
    uint16_t own_address;
} i2c_params_t;
}

class i2c {
public:
    /**
     * @brief   Constructor. Literally do nothing except initializing some data.
     */
    i2c(uint8_t used_i2c);

    /**
     * @brief   Initialize I2C module and corresponding gpios.
     *
     * @param[in]   init_structure, a struct holding parameters to initialize i2c module.
     */
    void init(i2c_ns::i2c_params_t *init_structure);

    /**
     * @brief   Disable i2c module and shutdown all gpios.
     */
    void deinit(void);

    /**
     * @brief   Get the used i2c
     *
     * @return  used i2c number.
     */
    uint8_t get_used_i2c(void);

    /**
     * @brief   Send a buffer of data to a slave and terminate the transaction with
     *          STOP signal.
     *
     * @param[in]   slave_addr, address a receiving slave.
     * @param[in]   send_buff, buffer holding data to be sent.
     * @param[in]   count, number of bytes in send_buff will be sent to slave.
     * @param[in]   stop_signal, true to generate STOP signal in the end of transaction.
     *
     */
    void master_send(uint16_t slave_7b_addr, uint8_t *send_buff, uint16_t size,
            bool stop_signal);

    /**
     * @brief   Initiate a transaction to receive data from a slave and terminate
     *          with STOP signal.
     *
     * @param[in]   slave_addr, address a slave.
     * @param[in]   slave_register, address to start reading from the slave.
     * @param[out]  recv_buff, buffer holding received data.
     * @param[in]   count, number of bytes need to be read from slave.
     */
    void master_receive(uint16_t slave_7b_addr, uint8_t slave_register,
            uint8_t *recv_buff, uint16_t size);
private:
    uint8_t used_i2c = 1; //I2C1 as default
};

#endif /* MB1_I2C_H_ */

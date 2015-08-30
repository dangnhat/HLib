/*
 * MB1_I2C.h
 *
 *  Created on: Aug 25, 2015
 *      Author: nvhien1992
 */

#ifndef MB1_I2C_H_
#define MB1_I2C_H_

/* Includes */
#include "MB1_Glb.h"

namespace I2C_ns {
const uint8_t num_of_I2Cs = 2;
}

class I2C {
public:
    /**
     * @brief
     */
    I2C(uint8_t used_i2c);

    /**
     * @brief
     */
    void reinit(I2C_InitTypeDef *init_structure);

    /**
     * @brief
     */
    void deinit(void);

    /**
     *
     */
    uint8_t get_used_i2c(void);

    /**
     * @brief
     */
    bool master_send_to(uint16_t slave_7b_addr, uint8_t *send_buff, uint16_t size,
            bool stop_signal);

    /**
     * @brief
     */
    bool master_receive_from(uint16_t slave_7b_addr, uint8_t *recv_buff,
            uint16_t size);

    /**
     * @brief
     */
    bool master_receive_from(uint16_t slave_7b_addr, uint8_t slave_register,
            uint8_t *recv_buff, uint16_t size);
private:
    uint8_t used_i2c = 1; //I2C1 as default
};

#endif /* MB1_I2C_H_ */

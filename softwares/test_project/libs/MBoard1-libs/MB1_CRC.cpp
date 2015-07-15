/**
 @file MB1_CRC.cpp
 @brief Implement some functions to calculate CRC and to check error from received CRC

 @author  Do Mai Anh Tu <aberrant.accolades@gmail.com>
 Nguyen Van Hien <nvhien1992@gmail.com>
 @version 1.1
 @date 1/11/2013
 @modified date 06/10/2014
 @copyright
 This project and all its relevant hardware designs, documents, source codes, compiled libraries
 belong to <b> Smart Sensing and Intelligent Controlling Group (SSAIC Group)</b>.
 You have to comply with <b> Non-Commercial Share-Alike Creative Common License </b>
 in order to share (copy, distribute, transmit) or remix (modify, reproduce, adapt) these works.\n
 SSAIC Group shall not be held liable for any direct, indirect or consequential damages
 with respect to any claims arising from the content of hardware, firmware and/or the use
 made by customers of the coding information contained herein in connection with their products.\n
 You are prohibited from commercializing in any kind that using or basing on these works
 without written permission from SSAIC Group. Please contact ssaic@googlegroups.com for commercializing
 */

#include "MB1_CRC.h"

void crc_hw::crc_start(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    CRC->CR = CRC_CR_RESET;
}

void crc_hw::crc_shutdown(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);
}

void crc_hw::crc_clear(void)
{
    CRC->CR = CRC_CR_RESET;
}

uint32_t crc_hw::reverse_bit(uint32_t data, uint16_t num_bits)
{
    uint32_t retval = 0;

    for (int i = 0; i < num_bits; i++) {
        retval |= ((data >> i) & 1) << (num_bits - 1 - i);
    }

    return retval;
}

uint32_t crc_hw::crc32_block_cal(uint8_t* data, uint16_t data_size)
{
    uint32_t *data_ptr = (uint32_t*) data;
    uint32_t crc32, data_tmp;
    uint32_t crc_reg = 0xFFFFFFFF;
    uint16_t len;

    crc_clear();

    /* Calculate CRC32 of 4*[integer] bytes in the given data pool */
    for (len = data_size; len >= 4; len = len - 4) {
        data_tmp = *data_ptr++;
        /* Reverse bit before calculating CRC */
        data_tmp = reverse_bit(data_tmp, 32);
        crc_reg = CRC_CalcCRC(data_tmp);
    }
    crc32 = reverse_bit(crc_reg, 32);

    /* If remaining bytes are less than 4bytes, keeping calculating CRC32 */
    if (len > 0) { //3, 2 or 1 byte.
        CRC_CalcCRC(crc_reg);
        data_tmp = (*data_ptr & (0xFFFFFFFF >> (32 - 8 * len))) ^ crc32;
        data_tmp = reverse_bit(data_tmp, 32);
        crc_reg = CRC_CalcCRC(data_tmp >> (32 - 8 * len));
        crc_reg = reverse_bit(crc_reg, 32);
        crc32 = (crc32 >> (8 * len)) ^ crc_reg;
    }

    return crc32 ^ 0xFFFFFFFF; //~crc32
}

bool crc_hw::crc_check(uint8_t *data, uint16_t data_size, uint32_t received_crc)
{
    return (received_crc == crc32_block_cal(data, data_size));
}


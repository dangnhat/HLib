/**
 @file MB1_CRC.h
 @brief Implement some prototype functions to calculate CRC and check error from CRC received

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

#ifndef __HL_CRC_H
#define __HL_CRC_H

#include "MB1_Glb.h"

/**
 * @brief Providing controlling method for CRC peripheral of STM32f1.
 * 	Standard: Ethernet, PKZip, AUTODIN II, FDDI.
 * 		CRC type: 			CRC32
 * 		Polynomial:			0x04C11DB7
 * 		Initial remainder: 	0xFFFFFFFF
 *		Reflect in: 		TRUE
 *		Reflect out:		TRUE
 *		Final XorOut: 		0xFFFFFFFF
 */
class crc_hw {
public:

    /**
     * @brief	Enable CRC clock, reset all registers to default values.
     * 			It must be called before calling other methods.
     *
     * @param[in]	none.
     * @param[out]	none.
     *
     * @return	none.
     */
    void crc_start(void);

    /**
     * @brief	Disable CRC clock.
     *
     * @param[in]	none.
     * @param[out]	none.
     *
     * @return	none.
     */
    void crc_shutdown(void);

    /**
     * @brief The method that can calculate CRC32 of the given data pool.
     *
     * @param[in]	data		A pointer point to a data pool.
     * @param[in]	data_size	The total size of the data pool in byte.
     *
     * return CRC32 of the data pool.
     */
    uint32_t crc32_block_cal(uint8_t *data, uint16_t data_size);

    /**
     * @brief The method used to check error on the given data pool with received CRC32.
     *
     * @param[in]	data	 		A pointer point to a data pool needed to check.
     * @param[in]	data_size		The total size of the given data in byte.
     * @param[in]	received_crc	Received CRC used to compare with calculated CRC on the given data.
     *
     * return	true if the received CRC is matched the calculated CRC, false otherwise.
     */
    bool crc_check(uint8_t *data, uint16_t data_size, uint32_t received_crc);

private:
    /**
     * @brief	Clear current calculating CRC value to zero.
     *
     * @param[in]	none.
     * @param[out]	none.
     *
     * @return	none.
     */
    void crc_clear(void);

    /**
     * @brief	Reverse the order of the bits in data.
     *
     * @param[in]	data		Data needed to reverse.
     * @param[in]	num_bits	Number of bits can be reversed in data.
     *
     * @return	The value of data after being reversed.
     */
    uint32_t reverse_bit(uint32_t data, uint16_t num_bits);
};
//end class

#endif //__HL_CRC_H

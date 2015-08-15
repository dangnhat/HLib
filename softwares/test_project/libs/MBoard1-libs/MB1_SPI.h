/**
 * @file MB1_SPI.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.4
 * @date 11-Aug-2015
 * @brief This is header file for SPIs on MBoard-1. MBoard-1 only supports SP1 and
 * SP2 as AF. SPI1 on APB2 clock brigde with Fmax = SystemClock, SP2 on APB1 with
 * Fmax = SystemClock/2.
 * How to use this lib:
 * - Declare an SPI instance.
 * ------ master mode ---------
 * - Set up numOfSSLines.
 * - Set up GPIO for NSS lines by calling SM_NSS_GPIOs_set.
 * - Set up device-to-decoder table by calling SM_deviceToDecoder_set (remember to set decode value for allFree).
 * - When using SPI :
 *  + init SPI.
 *  + attach SPI to a device.
 *  + do somethings.
 *  + after finished, release SPI, so other device can use.
 *
 * @History:
 * 1.4: changed to use gpio class for NSS lines and fixed some typos.
 *      Removed slave select decoding table inside this lib. Thus, users must defined
 *      their own tables, which must link to initial table inside this lib.
 */

#ifndef _MB1_SPI_H_
#define _MB1_SPI_H_

/* Includes */
#include "MB1_Glb.h"
#include "MB1_GPIO.h"
#include "MB1_Misc.h"

namespace SPI_ns{

/* config (compile-time), we should config at compile time. */
const uint8_t numOfSPIs = 2;
const uint8_t SSLines_max = 3;
const uint8_t SSDevices_max = 0x01 << SSLines_max;

/* config (compile-time), we should config at compile time. */

/* SPI_global */
typedef enum: uint8_t {
    successful,
    failed,
    busy,
    decodeValueNotFound

} status_t;

/* end SPI_global */


/* conf (run-time) typedefs and global vars */
typedef struct {
    uint16_t baudRatePrescaler;     //SPI_BaudRatePrescaler_X, where X can be : 2,4,8,16,32,64,128,256
    uint16_t CPHA;                  //SPI_CPHA_1Edge or SPI_CPHA_2Edge
    uint16_t CPOL;                  //SPI_CPOL_High or SPI_CPOL_Low
    uint16_t crcPoly;
    uint16_t dataSize;              //SPI_DataSize_16b or SPI_DataSize_8b
    uint16_t direction;             //SPI_Direction_1Line_Rx, SPI_Direction_1Line_Tx, SPI_Direction_2Lines_FullDuplex, SPI_Direction_2Lines_RxOnly
    uint16_t firstBit;              //SPI_FirstBit_LSB or SPI_FirstBit_MSB
    uint16_t mode;                  //SPI_Mode_Master, SPI_Mode_Slave
    uint16_t nss;                   //SPI_NSS_Hard, SPI_NSS_Soft.
} SPI_params_t;

enum buadrate_prescaler_e: uint16_t {
    bp2 = SPI_BaudRatePrescaler_2,
    bp4 = SPI_BaudRatePrescaler_4,
    bp8 = SPI_BaudRatePrescaler_8,
    bp16 = SPI_BaudRatePrescaler_16,
    bp32 = SPI_BaudRatePrescaler_32,
    bp64 = SPI_BaudRatePrescaler_64,
    bp128 = SPI_BaudRatePrescaler_128,
    bp256 = SPI_BaudRatePrescaler_256,
};

enum CPHA_e: uint16_t {
    cpha_1edge = SPI_CPHA_1Edge,
    cpha_2edge = SPI_CPHA_2Edge,
};

enum CPOL_e: uint16_t {
    cpol_low = SPI_CPOL_Low,
    cpol_high = SPI_CPOL_High,
};

enum datasize_e: uint16_t {
    d8b = SPI_DataSize_8b,
    d16b = SPI_DataSize_16b,
};

enum direction_e: uint16_t {
    dr_1line_rx = SPI_Direction_1Line_Rx,
    dr_1line_tx = SPI_Direction_1Line_Tx,
    dr_2lines_fullduplex = SPI_Direction_2Lines_FullDuplex,
    dr_2lines_rxonly = SPI_Direction_2Lines_RxOnly,
};

enum mode_e: uint16_t {
    master = SPI_Mode_Master,
    slave = SPI_Mode_Slave,
};

enum firstbit_e: uint16_t {
    msb_first = SPI_FirstBit_MSB,
    lsb_first = SPI_FirstBit_LSB,
};

enum nss_mode_e: uint16_t {
    soft_nss = SPI_NSS_Soft,
    hard_nss = SPI_NSS_Hard,
};

/* end conf (run-time) */

/* -------------- master mode --------------------------------*/

/* slave_mgr interface */
enum sm_decoding_table_e: uint16_t {
    all_free,

    last_of_initial_table,
};

/* end slave_mgr */

/* -------------- master mode --------------------------------*/
}

class SPI {

public:
    SPI (uint16_t usedSPI);

    /* conf (run-time) interface */
    SPI_ns::status_t init (const SPI_ns::SPI_params_t *params_struct);
    void deinit(void);

    /* end conf (run-time) */

    /* -------------- master mode --------------------------------*/

    /* slave_mgr interface (soft NSS) */

    /* conf (run-time) */
    SPI_ns::status_t SM_numOfSSLines_set (uint8_t numOfSSLines);
    SPI_ns::status_t SM_GPIO_set (gpio* gpio_p, uint8_t ss_line);
    SPI_ns::status_t SM_deviceToDecoder_set (uint16_t device, uint8_t decode_value);
    void SM_deinit(void);
    /* conf (run-time) */

    SPI_ns::status_t SM_device_attach (uint16_t device);
    SPI_ns::status_t SM_device_release (uint16_t device);

    SPI_ns::status_t SM_device_select (uint16_t device);
    SPI_ns::status_t SM_device_deselect (uint16_t device);

    /* end slave_mgr */

    /* master 2 lines, full duplex interface */
    uint16_t M2F_sendAndGet_blocking (uint16_t device, uint16_t data);

    /* master 2 lines, full duplex interface */

    /* misc functions */
    uint8_t misc_MISO_read (void);
    uint16_t get_usedSPI(void) {return usedSPI + 1;}
    /* misc functions */

    /* -------------- master mode --------------------------------*/

private:
    uint16_t usedSPI;
    uint16_t spi_direction = 0;

    /* app_conf config at run-time by methods */
    gpio *soft_nss_gpios [SPI_ns::SSLines_max];

    void M2F_GPIOs_Init (void);
    void M2F_GPIOs_Deinit (void);
    /* end app_conf */

    /* -------------- master mode --------------------------------*/

    /* slave_mgr interface */
    uint16_t SS_pins_set;             //It's used to know which SSPin has been set or not, bit 0 is for HardNSS pin.
    uint8_t SM_numOfNSSLines;
    uint8_t SM_numOfDevices; // always = 2^SM_numOfNSSLines

    uint16_t SM_deviceToDecoder_table [SPI_ns::SSDevices_max];
    uint16_t SM_deviceInUse;
    uint8_t SM_decodeValueInUse; // use to select a device, only change when device_in_use change in attach fucntion.
    uint8_t SM_decode_all_free; // use when deselect a device, it set only after set decode value for SPI_allFree.

    SPI_ns::status_t SM_decodeValueInUse_update (void);

    /* end slave_mgr */

    /* -------------- master mode --------------------------------*/
};


#endif // _MB1_SPI_H_

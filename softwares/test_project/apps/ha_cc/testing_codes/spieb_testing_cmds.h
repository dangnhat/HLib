/**
 * @file spieb_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 26-Aug-2015
 * @brief Header file for SPI EBoard testing shell commands.
 */

#ifndef SPIEB_TESTING_CMDS_H_
#define SPIEB_TESTING_CMDS_H_

/**
 * @brief   SPI EBoard testing cmd.
 *
 * @details Usage:
 *      spieb_test -i, initialize hardware and data for the test.
 *      spieb_test -d, deinitialize hardware and data.
 *      spieb_test -e, SPI EEPROM communication test.
 *      spieb_test -r, Digital potentiometer communication test.
 *      spieb_test -t, Temperature IC communication test.
 *      spieb_test -f, perform full test = spieb_test -i -e -r -t -d.
 *      spieb_test -h, print the usage.
 *      Press ESC to stop the test.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void spieb_test(int argc, char** argv);

#endif /* SPIEB_TESTING_CMDS_H_ */

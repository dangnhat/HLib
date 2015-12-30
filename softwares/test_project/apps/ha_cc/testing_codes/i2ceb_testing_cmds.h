/**
 * @file i2ceb_testing_cmds.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 30-Aug-2015
 * @brief Header file for I2C EBoard testing shell commands.
 */

#ifndef I2CEB_TESTING_CMDS_H_
#define I2CEB_TESTING_CMDS_H_

/**
 * @brief   I2C EBoard testing cmd.
 *
 * @details Usage:
 *      i2ceb_test -i, initialize hardware and data for the test.
 *      i2ceb_test -d, deinitialize hardware and data.
 *      i2ceb_test -e, I2C EEPROM communication test.
 *      i2ceb_test -w, 1-wired IC communication test.
 *      i2ceb_test -l, Light sensor IC communication test.
 *      i2ceb_test -t, Temperature IC communication test.
 *      i2ceb_test -f, perform full test = spieb_test -i -e -w -l -t -d.
 *      i2ceb_test -h, print the usage.
 *      Press ESC to stop the test.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void i2ceb_test(int argc, char** argv);

#endif /* I2CEB_TESTING_CMDS_H_ */
